#ifndef __services_main_cpp__
#define __services_main_cpp__

#include <variant>
#include "services_main.hpp"

Services::Services(const std::int32_t& _qsize, const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, 
                    const std::string& _localHost, const std::uint16_t& _localPort) {
    m_epollFd = -1;
    m_httpServer = std::make_shared<HTTPServer>(_qsize,_protocol,_blocking, _ipv4, _localHost, _localPort);
}

Services::Services(const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, const std::string& _localHost, const std::uint16_t& _localPort) {
    m_httpClient = std::make_shared<HTTPClient>(_protocol, _blocking, _ipv4, _localHost, _localPort);
}

Services& Services::init() {
    if(m_epollFd < 0) {
        m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
    }
    return(*this);
}

Services& Services::start() {

    return(*this);
}

Services& Services::stop() {

    return(*this);
}

Services::~Services() {

}

std::shared_ptr<HTTPServer> Services::httpServer() const {
    return(m_httpServer);
}

std::shared_ptr<HTTPClient> Services::httpClient() const {
    return(m_httpClient);
}

int main(std::int32_t argc, char* argv[]) {
    

    Services svc_HttpServer(10, IPPROTO_TCP, true, true, "0.0.0.0", 58989);
    Services svc_HttpClient(IPPROTO_TCP, true, true);
    

    return(0);
}


#endif /*__services_main_cpp__*/