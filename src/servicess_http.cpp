#ifndef __services_http_cpp__
#define __services_http_cpp__

#include "servicess_http.hpp"

HTTPServer::HTTPServer(const std::int32_t& _qsize, const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, const std::string& _localHost, const std::uint16_t& _localPort)
            : Server(_qsize, _protocol, _blocking, _ipv4, _localHost, _localPort) {

}

HTTPServer::~HTTPServer() {

}

std::int32_t HTTPServer::onReceive(std::vector<char> out, std::int32_t out_len) {

    return(0);
}

HTTPClient::HTTPClient(const std::int32_t& _protocol, const bool& _blocking, const bool& _ipv4, const std::string& _localHost, const std::uint16_t& _localPort)
            : Client(_protocol, _blocking, _ipv4, _localHost, _localPort) {

}

HTTPClient::~HTTPClient() {

}

std::int32_t HTTPClient::onReceive(std::vector<char> out, std::int32_t out_len) {

    return(0);
}


#endif /*__services_http_hpp_cpp__*/