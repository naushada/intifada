#ifndef __services_main_hpp__
#define __services_main_hpp__

#include <memory>
#include <unordered_map>

#include "servicess_http.hpp"
#include "json.hpp"

extern "C" {
    #include <sys/epoll.h>
    #include <getopt.h>
    #include <sys/signalfd.h>
    #include <signal.h>
    
}

using json = nlohmann::json;

class Services {
    public:
        enum ServiceType:std::uint8_t {ServiceServer=0, ServiceClient=1, ServiceConnectedClient=2, ServiceTimer=3, 
                          ServiceSignal=4, ServiceNotifier=5, ServiceInvalid=6};
        enum ServiceApplicationProtocolType:std::uint8_t {REST=0, LwM2M=1, ProtocolInvalid=2};
        enum ServiceTransportType:std::uint8_t {TCP=0, UDP=1, TransportInvalid=2};
        enum ServiceSecurityType:std::uint8_t {TLS=0, DTLS=1, SecurityNone=2};

        Services();
        Services& init();
        Services& start();
        Services& stop();
        std::int32_t handleClientConnection(std::int32_t handle, ServiceType st, ServiceApplicationProtocolType sap, ServiceTransportType stt, ServiceSecurityType sst);
        std::int32_t handleIO(std::int32_t handle, ServiceType st, ServiceApplicationProtocolType sap, ServiceTransportType stt, ServiceSecurityType sst);
        virtual ~Services();

        std::int32_t createHttpServer(const std::int32_t& _qsize, const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, 
                    const std::string& _localHost, const std::uint16_t& _localPort);

        std::int32_t createNotifierClient(const std::int32_t& _protocol, const bool& blocking, const bool& _ipv4, 
                 const std::string& _peerHost, const std::uint16_t& _peerPort, const std::string& _localHost, const std::uint16_t& _localPort);

        std::int32_t createRestClient(const std::int32_t& _protocol, const bool& blocking, const bool& _ipv4, 
                 const std::string& _peerHost, const std::uint16_t& _peerPort, const std::string& _localHost, const std::uint16_t& _localPort);

        std::int32_t deleteClient(ServiceType st);
        std::shared_ptr<HTTPServer> httpServer() const;
        std::shared_ptr<HTTPClient> notifierClient() const;
        std::shared_ptr<HTTPClient> restClient() const;
        void userid(std::string uid);
        void password(std::string pwd);
        std::string userid() const;
        std::string password() const;

    private:
        std::int32_t m_epollFd;
        std::shared_ptr<HTTPServer> m_httpServer;
        std::shared_ptr<HTTPClient> m_notifierClient;
        std::shared_ptr<HTTPClient> m_restClient;
        std::vector<struct epoll_event> m_events;
        sigset_t m_mask;
        std::string m_userid;
        std::string m_password;
};








#endif /*__services_main_hpp__*/