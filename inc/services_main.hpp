#ifndef __services_main_hpp__
#define __services_main_hpp__

#include <memory>
#include "servicess_http.hpp"

extern "C" {
    #include <sys/epoll.h>
    #include <getopt.h>
}

class Services {
    public:

        Services(const std::int32_t& _qsize, const std::int32_t& _protocol, const bool& blocking, const bool& _ipv4, 
                 const std::string& _localHost="0.0.0.0", const std::uint16_t& _localPort=0);

        Services(const std::int32_t& _protocol, const bool& blocking, const bool& _ipv4, 
                 const std::string& _localHost="0.0.0.0", const std::uint16_t& _localPort=0);

        Services& init();
        Services& start();
        Services& stop();

        virtual ~Services();

        std::shared_ptr<HTTPServer> httpServer() const;
        std::shared_ptr<HTTPClient> httpClient() const;

    private:
        std::int32_t m_epollFd;
        std::shared_ptr<HTTPServer> m_httpServer;
        std::shared_ptr<HTTPClient> m_httpClient;
        
};








#endif /*__services_main_hpp__*/