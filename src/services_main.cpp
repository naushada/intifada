#ifndef __services_main_cpp__
#define __services_main_cpp__

#include <variant>
#include <vector>
#include <algorithm>
#include <cerrno>
#include <cstring>

#include "services_main.hpp"
#include "command_line_argument.hpp"

Services::Services(): m_epollFd(-1), m_notifierClient(nullptr), m_httpServer(nullptr), m_restClient(nullptr) {
    if(m_epollFd < 0) {
        m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
    }
}

std::int32_t Services::handleClientConnection(std::int32_t handle, ServiceType st, ServiceApplicationProtocolType sap, ServiceTransportType stt, ServiceSecurityType sst) {
    return(0);
}

std::int32_t Services::deleteClient(ServiceType st) {
    switch (st)
    {
    case ServiceType::ServiceNotifier:
    {
        std::int32_t channel = notifierClient()->handle();
        std::cout <<__FUNCTION__<<":"<<__LINE__<<"Notifier Client is closed for channel: " << channel << std::endl;
        ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, channel, nullptr);
        std::remove_if(m_events.begin(), m_events.end(), [&](auto& ent) -> bool {
            return(channel == static_cast<std::int32_t>(((ent.data.u64 & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF));
        });
        m_notifierClient.reset();
        m_restClient = nullptr;
    }
        /* code */
        break;
    case ServiceType::ServiceClient:
    {
        std::int32_t channel = restClient()->handle();
        std::cout <<__FUNCTION__<<":"<<__LINE__ <<"Rest Client is closed for channel:" << channel << std::endl;
        ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, channel, nullptr);
        std::remove_if(m_events.begin(), m_events.end(), [&](auto& ent) -> bool {
            return(channel == static_cast<std::int32_t>(((ent.data.u64 & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF));
        });
        m_restClient.reset();
        m_restClient = nullptr;
    }
        /* code */
        break;
    default:
        break;
    }
}

std::int32_t Services::handleIO(std::int32_t channel, ServiceType st, ServiceApplicationProtocolType sap, ServiceTransportType stt, ServiceSecurityType sst) {
    std::string out;
    if(ServiceType::ServiceServer == st) {
        /// @brief New Connection , accept it.
        std::int32_t Fd = httpServer()->accept();
        if(Fd > 0) {
            struct epoll_event evt;
            evt.events = EPOLLIN;
            evt.data.u64 = (((static_cast<std::uint64_t>(Fd & 0xFFFFFFFFU)) << 32U) |
                              static_cast<std::uint64_t>(((ServiceType::ServiceConnectedClient & 0xFFU) << 24U) |
                                                         ((ServiceApplicationProtocolType::REST & 0xFFU) << 16U) |
                                                         ((ServiceSecurityType::SecurityNone & 0xFFU) << 8U)) |
                                                         (ServiceTransportType::TransportInvalid & 0xFF));
            ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, Fd, &evt);
            m_events.push_back(evt);
            std::cout << __FUNCTION__ <<":"<< __LINE__<<":" <<"New Notifier is connected on channel:" << Fd << std::endl;
        }
    } else if(ServiceType::ServiceConnectedClient == st) {
        /// @brief Data is received, read it.

        std::cout << __FUNCTION__ <<":"<< __LINE__<<":" << "Data from Notifier is received on channel:" << channel << std::endl;
        auto clnt = httpServer()->get_client(channel);

        if(nullptr == clnt) {
            std::cout <<__FUNCTION__ <<":"<< __LINE__ << "Unable to get the client for channel:"<< channel << std::endl;
            return(-1);
        }

        out.reserve(1024);
        //auto ret = clnt->rx(out);
        auto ret = ::recv(channel, (void*)out.data(), out.length(), 0);
        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"From Notifier ret: "<< ret << std::endl;
        
        if(ret > 0) {
            out.resize(ret);
            std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Received: "<< out << "out.len:" << out.length()<<std::endl;
            httpServer()->onReceive(out);
        } else if(!ret) {
            std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connected Notifier Client is closed: "<< std::endl;
            ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, channel, nullptr);
            std::remove_if(m_events.begin(), m_events.end(), [&](auto& ent) -> bool {
                return(channel == static_cast<std::int32_t>(((ent.data.u64 & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF));
            });

            httpServer()->remove_client(channel);
        } else {
            std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Error on channel: "<< channel << " error string->" << std::strerror(errno) << std::endl;
        }
    } else if(ServiceType::ServiceNotifier == st) {
        /// @brief Data received from Notifier Server.
        auto ret = notifierClient()->rx(out);
        if(!ret) {
            /// @brief Server is down or restarted.
            std::string peerHost = notifierClient()->peerHost();
            std::string localHost = notifierClient()->peerHost();
            std::uint16_t peerPort = notifierClient()->peerPort();
            std::uint16_t localPort = notifierClient()->localPort();

            deleteClient(st);
            ///@brief Attempt to Connect to Command & Control Centre.
            createNotifierClient(IPPROTO_TCP, true, true, peerHost, peerPort, localHost, localPort);

        } else {
            /// @brief Received From Command & Control for Notifier.
            notifierClient()->onReceive(out);
        }
    } else if(ServiceType::ServiceClient == st) {
        /// @brief Data received from REST Server.
        if(ServiceSecurityType::TLS == sst) {
            std::string request;
            std::size_t len = 1024;
            std::size_t actualLength = restClient()->tls()->read(request, len);
            std::stringstream ss;
            if(!actualLength) {
                /// @brief Peer has closed the connection
                /// @brief Server is down or restarted.
                std::string peerHost = restClient()->peerHost();
                std::string localHost = restClient()->peerHost();
                std::uint16_t peerPort = restClient()->peerPort();
                std::uint16_t localPort = restClient()->localPort();

                deleteClient(st);
                ///@brief Attempt to Connect to Command & Control Centre.
                createRestClient(IPPROTO_TCP, true, true, peerHost, peerPort, localHost, localPort);
            } else {
                Http http(request);
                //std::cout << __FUNCTION__ <<":" <<__LINE__ <<"Request:" << request << std::endl; 
                std::int32_t effectiveLength = http.header().length() - actualLength;
                if(http.value("Content-Length").length()) {
                    ///Content-Length Field is present.
                    effectiveLength = (http.header().length() + std::stoi(http.value("Content-Length"))) - actualLength;
                }
                ss << request;
                std::int32_t offset = 0;
                while(offset != effectiveLength) {
                    len = restClient()->tls()->read(request, effectiveLength - offset);
                    if(len > 0) {
                        ss << request;
                    }
                    
                    if(len < 0) {
                        ///Error handling
                        std::cout << basename(__FILE__) <<":" << __LINE__ << " Error recv failed" << std::endl;
                        break;
                    }
                    offset += len;
                }
                restClient()->onReceive(ss.str());
                if(HTTPClient::HTTPUriName::GetChangeEventsNotification == restClient()->sentURI()) {
                    ///@brief Build payload and send to Notifier Server.
                    
                    if(notifierClient()) {
                        auto notify = json::object();
                        notify = {
                            {"endPoint", restClient()->serialNumber()},
                            {"latitude", restClient()->latitude()},
                            {"longitude", restClient()->longitude()}
                        };

                        auto req = restClient()->buildHeader(HTTPClient::HTTPUriName::NotifyLocation, notify.dump());
                        auto len = notifierClient()->tx(req);
                        std::cout <<__FUNCTION__<<":" << __LINE__<<"Sent to Notifier len:" << len << std::endl << req << std::endl;
                    }
                }
            }
        } else {
            std::int32_t len = out.size();
            auto ret = restClient()->rx(out);
            if(!ret) {
                /// @brief Server is down or restarted.
                std::string peerHost = restClient()->peerHost();
                std::string localHost = restClient()->peerHost();
                std::uint16_t peerPort = restClient()->peerPort();
                std::uint16_t localPort = restClient()->localPort();

                deleteClient(st);
                ///@brief Attempt to Connect to Command & Control Centre.
                createRestClient(IPPROTO_TCP, true, true, peerHost, peerPort, localHost, localPort);
            } else if(ret > 0) {
                restClient()->onReceive(out);
            }
        }
    } else if(ServiceType::ServiceSignal == st) {
        std::cout << __FUNCTION__ <<":"<< __LINE__<<":"<<"Signal is Pressed" << std::endl;
        ::recv(channel, (void*)out.data(), 128, 0);
    }
}
std::int32_t Services::createHttpServer(const std::int32_t& _qsize, const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, 
                                        const std::string& _localHost, const std::uint16_t& _localPort) {
    m_httpServer = std::make_shared<HTTPServer>(_qsize,_protocol,_blocking, _ipv4, _localHost, _localPort);
    std::int32_t channel = m_httpServer->handle();
    if(channel > 0) {
        struct epoll_event evt;
        evt.events = EPOLLHUP | EPOLLIN;
        evt.data.u64 = (((static_cast<std::uint64_t>(channel & 0xFFFFFFFFU)) << 32U) |
                          static_cast<std::uint64_t>(((ServiceType::ServiceServer & 0xFFU) << 24U) |
                                                     ((ServiceApplicationProtocolType::REST & 0xFFU) << 16U) |
                                                     ((ServiceSecurityType::SecurityNone & 0xFFU) << 8U)) |
                                                     (ServiceTransportType::TCP & 0xFF));
        ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, channel, &evt);
        m_events.push_back(evt);
    }
}

std::int32_t Services::createNotifierClient(const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, 
                                        const std::string& _peerHost, const std::uint16_t& _peerPort, const std::string& _localHost, const std::uint16_t& _localPort) {
    m_notifierClient = std::make_shared<HTTPClient>(_protocol, _blocking, _ipv4, _peerHost, _peerPort, _localHost, _localPort);
    
    std::int32_t conErr =  notifierClient()->connect();
    struct epoll_event evt;

    if(!conErr) {
        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connected to Notifier peer->" << notifierClient()->peerHost() << ":" << notifierClient()->peerPort() << std::endl;
        evt.events = EPOLLHUP | EPOLLIN;
    } else if(conErr == EINPROGRESS) {
        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connecting... to Notifier peer->" << notifierClient()->peerHost() << ":" << notifierClient()->peerPort() << std::endl;
        evt.events = EPOLLHUP | EPOLLOUT;
    } else {
        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Error... to Notifier peer->" << notifierClient()->peerHost() << ":" << notifierClient()->peerPort() << std::endl;
    }

    std::int32_t channel = notifierClient()->handle();
        
    evt.data.u64 = (((static_cast<std::uint64_t>(channel & 0xFFFFFFFFU)) << 32U) |
                      static_cast<std::uint64_t>(((ServiceType::ServiceNotifier & 0xFFU) << 24U) |
                                                  ((ServiceApplicationProtocolType::REST & 0xFFU) << 16U) |
                                                  ((ServiceSecurityType::SecurityNone & 0xFFU) << 8U)) |
                                                   (ServiceTransportType::TCP & 0xFF));
        
    ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, channel, &evt);
    m_events.push_back(evt);

}

std::int32_t Services::createRestClient(const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, 
                                        const std::string& _peerHost, const std::uint16_t& _peerPort, const std::string& _localHost, const std::uint16_t& _localPort) {
    m_restClient = std::make_shared<HTTPClient>(_protocol, _blocking, _ipv4, _peerHost, _peerPort, _localHost, _localPort);
    std::int32_t conErr =  restClient()->connect();
    struct epoll_event evt;

    //if(EINPROGRESS !=conErr) {
    if(!conErr) {
        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connected to peer->" << restClient()->peerHost() << ":" << restClient()->peerPort() << std::endl;
        /// @brief -- Use TLS now.
        restClient()->tls()->init(restClient()->handle());
        /// @brief initiate TLS handshake now.
        restClient()->tls()->client();
        /// @brief Get the AUTH Token from REST Server.
        //auto req = restClient()->buildGetTokenRequest(userid(), password());
        auto req = restClient()->buildGetTokenRequest("user", "Pin@411048");
        std::cout << "REQ:"<<std::endl << req << std::endl;
        if(!req.empty()) {
            auto len = restClient()->tls()->write(req, req.length());
            std::cout << __FUNCTION__ <<":"<<__LINE__<<" Sent to REST Server len:" << len << std::endl;
            restClient()->sentURI(HTTPClient::GetTokenForSession);
        }
        evt.events = EPOLLHUP | EPOLLIN;
    } else {
        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connecting... to peer->" << restClient()->peerHost() << ":" << restClient()->peerPort() << std::endl;
        evt.events = EPOLLHUP | EPOLLOUT;
    }

    std::int32_t channel = restClient()->handle();
        
    evt.data.u64 = (((static_cast<std::uint64_t>(channel & 0xFFFFFFFFU)) << 32U) |
                      static_cast<std::uint64_t>(((ServiceType::ServiceClient & 0xFFU) << 24U) |
                                                ((ServiceApplicationProtocolType::REST & 0xFFU) << 16U) |
                                                ((ServiceSecurityType::TLS & 0xFFU) << 8U)) |
                                                (ServiceTransportType::TCP & 0xFF));

    ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, channel, &evt);
    m_events.push_back(evt);

}

Services& Services::init() {
    /// @brief Adding Signal Processing.
    sigemptyset(&m_mask);
    sigaddset(&m_mask, SIGINT);
    sigaddset(&m_mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &m_mask, NULL);
    
    std::int32_t sfd = signalfd(-1, &m_mask, SFD_NONBLOCK);
    if(sfd > 0) {
        struct epoll_event evt;
        evt.events = EPOLLHUP | EPOLLIN;
        evt.data.u64 = (((static_cast<std::uint64_t>(sfd & 0xFFFFFFFFU)) << 32U) |
                          static_cast<std::uint64_t>(((ServiceType::ServiceSignal & 0xFFU) << 24U) |
                                                     ((ServiceApplicationProtocolType::ProtocolInvalid & 0xFFU) << 16U) |
                                                     ((ServiceSecurityType::SecurityNone & 0xFFU) << 8U)) |
                                                     (ServiceTransportType::TransportInvalid & 0xFF));
        ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, sfd, &evt);
        m_events.push_back(evt);
    }

    return(*this);
}

Services& Services::start() {
    std::vector<struct epoll_event> events;
    bool infinite = true;
#if 0
    if(nullptr != httpClient()) {
        std::int32_t conErr =  httpClient()->connectAsync();
        std::cout << " Connecting to peer->" << httpClient()->peerHost() << ":" << httpClient()->peerPort() << std::endl;
        if(EINPROGRESS ==conErr) {
            struct epoll_event evt;
            std::int32_t channel = httpClient()->handle();
            struct epoll_event evt;
            evt.events = EPOLLHUP | EPOLLOUT;
            evt.data.u64 = (((static_cast<std::uint64_t>(channel & 0xFFFFFFFFU)) << 32U) |
                            static_cast<std::uint64_t>(((ServiceType::ServiceClient & 0xFFU) << 24U) |
                                                        ((ServiceApplicationProtocolType::REST & 0xFFU) << 16U) |
                                                        ((ServiceSecurityType::SecurityNone & 0xFFU) << 8U)) |
                                                        (ServiceTransportType::TCP & 0xFF));
            ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, channel, &evt);
            m_events.push_back(evt);
        }
    }
#endif

    while(infinite) {
        events.resize(m_events.size());
        auto eventCount = ::epoll_wait(m_epollFd, events.data(), events.size(), 2000);

        if(!eventCount) {
            /// @brief This is a period of inactivity.
        } else if(eventCount > 0) {
            ///event is received.
            events.resize(eventCount);

            for(const auto& event: events) {
                struct epoll_event ent = event;
                auto elm = ent.data.u64;

                auto handle = (elm >> 32) & 0xFFFFFFFFU;
                ServiceType st = static_cast<ServiceType>((elm & 0xFF000000U) >> 24U);
                ServiceApplicationProtocolType sap = static_cast<ServiceApplicationProtocolType>((elm & 0x00FF0000U) >> 16U);
                ServiceTransportType stt = static_cast<ServiceTransportType>((elm & 0x0000FF00U) >> 8U);
                ServiceSecurityType sst = static_cast<ServiceSecurityType>(elm & 0x000000FFU);

#if 0
                std::cout << basename(__FILE__)<< ":" << __LINE__ << " channel:" << handle << " ServiceType:"
                          << st << " ServiceApplicationProtocolType:" << sap << " ServiceTransportType:" << stt 
                         << " ServiceSecurityType: " << sst << std::endl;
#endif

                if(ServiceType::ServiceSignal == st) {
                    std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Signal is received" << std::endl;
                    infinite = false;
                }

                if(ent.events & EPOLLOUT) {
                    //std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"ent.events: EPOLLOUT" << std::endl;
                    if(!handleClientConnection(handle, st, sap, stt, sst)) {
                        if(ServiceType::ServiceClient == st) {
                            std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connected to REST Server" << std::endl;
                            ent.events = EPOLLHUP|EPOLLIN;
                            ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, handle, &ent);
                        } else if(ServiceType::ServiceNotifier == st) {
                            std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"Connected to Notifier Server" << std::endl;
                            ent.events = EPOLLHUP|EPOLLIN;
                            ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, handle, &ent);
                        }
                    }
                }

                if(ent.events & EPOLLHUP) {
                    ///connection is closed by peer and read on socket will return 0 byte.
                    ///std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"ent.events: EPOLLHUP" << std::endl;
                    if(ServiceType::ServiceClient == st) {
                        /// @brief Server is down or restarted.
                        std::string peerHost = restClient()->peerHost();
                        std::string localHost = restClient()->peerHost();
                        std::uint16_t peerPort = restClient()->peerPort();
                        std::uint16_t localPort = restClient()->localPort();

                        deleteClient(st);
                        ///@brief Attempt to Connect to Command & Control Centre.
                        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"ent.events: EPOLLHUP Attempting connection to REST Server" << std::endl;
                        createRestClient(IPPROTO_TCP, true, true, peerHost, peerPort, localHost, localPort);
                    } else if(ServiceType::ServiceNotifier == st) {
                        std::string peerHost = notifierClient()->peerHost();
                        std::string localHost = notifierClient()->peerHost();
                        std::uint16_t peerPort = notifierClient()->peerPort();
                        std::uint16_t localPort = notifierClient()->localPort();

                        deleteClient(st);
                        ///@brief Attempt to Connect to Command & Control Centre.
                        std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"ent.events: EPOLLHUP Attempting connection to Notifier Server" << std::endl;
                        createNotifierClient(IPPROTO_TCP, true, true, peerHost, peerPort, localHost, localPort);
                    }
                }

                if(ent.events & EPOLLIN) {
                    //std::cout << "ent.events: EPOLLIN" << std::endl;
                    handleIO(handle, st, sap, stt, sst);
                }
                if(ent.events & EPOLLERR) {
                    ///Error on socket.
                    std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"ent.events: EPOLLERR" << std::endl;
                }
                if(ent.events & EPOLLET) {
                    std::cout <<__FUNCTION__ <<":"<< __LINE__<<":"<<"ent.events: EPOLLET" << std::endl;
                }
            }
        }
    }

    return(*this);
}

Services& Services::stop() {
    if(m_notifierClient) {
        m_notifierClient.reset();
    }
    if(m_httpServer) {
        m_httpServer.reset();
    }
    if(m_restClient) {
        m_restClient.reset();
    }
    return(*this);
}

Services::~Services() {

}

std::shared_ptr<HTTPServer> Services::httpServer() const {
    return(m_httpServer);
}

std::shared_ptr<HTTPClient> Services::notifierClient() const {
    return(m_notifierClient);
}

std::shared_ptr<HTTPClient> Services::restClient() const {
    return(m_restClient);
}

void Services::userid(std::string uid) {
    m_userid = uid;
}

void Services::password(std::string pwd) {
    m_password = pwd;
}

std::string Services::userid() const {
    return(m_userid);
}

std::string Services::password() const {
    return(m_password);
}

int main(std::int32_t argc, char* argv[]) {
    Services svcInst;
    std::string out;
    std::uint16_t port;
    Value value;
    CommandLineArgument options(argc, argv);
    if(options.getValue("role", value) && value.getString(out) && !out.empty()) {

        if(!out.compare("client") && options.getValue("peer-host", value) && value.getString(out) && !out.empty()) {

            options.getValue("peer-port", value);
            value.getUint16(port);
            std::cout << "peer-host:" << out << " peer-port:" << std::to_string(port) << std::endl;
            svcInst.createNotifierClient(IPPROTO_TCP, true, true, out, port, "0.0.0.0", 0);
            svcInst.createRestClient(IPPROTO_TCP, true, true, "192.168.1.1", 443, "0.0.0.0", 0);

            if(options.getValue("userid", value) && value.getString(out)) {
                svcInst.userid(out);
            }

            if(options.getValue("password", value) && value.getString(out)) {
                svcInst.password(out);
            }

        } else if(!out.compare("server") && options.getValue("local-host", value) && value.getString(out) && !out.empty()) {

            options.getValue("local-port", value);
            value.getUint16(port);
            std::cout << "local-host:" << out << " local-port:" << std::to_string(port) << std::endl;
            svcInst.createHttpServer(10, IPPROTO_TCP, true, true, out, port);

        } else if(!out.compare("both")) {


        } else {
            /// @brief TODO:
        }
    }
    
    svcInst.init().start().stop();

    return(0);
}


#endif /*__services_main_cpp__*/