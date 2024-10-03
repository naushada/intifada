#ifndef __services_http_hpp__
#define __services_http_hpp__

#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>

#include "services.hpp"

class HTTPServer : public Server {
    public:
        HTTPServer(const std::int32_t& _qsize, const std::int32_t& _protocol, const bool& blocking, const bool& _ipv4, 
                    const std::string& _localHost="0.0.0.0", const std::uint16_t& _localPort=0);
        virtual ~HTTPServer();
        virtual std::int32_t onReceive(std::vector<char> out, std::int32_t out_len) override;
    private:
};

class HTTPClient : public Client {
    public:
        HTTPClient(const std::int32_t& _protocol, const bool& blocking, const bool& _ipv4, 
                   const std::string& _localHost="0.0.0.0", const std::uint16_t& _localPort=0);
        virtual ~HTTPClient();
        virtual std::int32_t onReceive(std::vector<char> out, std::int32_t out_len) override;
    private:
};


#endif /*__services_http_hpp__*/