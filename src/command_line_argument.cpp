#ifndef __command_line_argument_cpp__
#define __command_line_argument_cpp__


#include "command_line_argument.hpp"

std::vector<struct option> options = {
    {"role",       required_argument, 0, 'r'},
    {"peer-host",  required_argument, 0, 'h'},
    {"peer-port",  required_argument, 0, 'p'},
    {"local-host", required_argument, 0, 'l'},
    {"locap-port", required_argument, 0, 'o'},
    {"protocol",   required_argument, 0, 't'},
    {"userid",     required_argument, 0, 'u'},
    {"password",   required_argument, 0, 'a'},
    {"timeout",    required_argument, 0, 'm'},
    {"long-poll",  required_argument, 0, 'n'}
};


CommandLineArgument::CommandLineArgument(std::int32_t argc, char* argv[]) {
    parseOptions(argc, argv);
}

bool CommandLineArgument::parseOptions(std::int32_t argc, char* argv[]) {
    std::int32_t c;
    std::int32_t option_index = 0;
    
    Value value;
    
    while ((c = getopt_long(argc, argv, "r:h:p:l:o:t:u:a:m:n:", options.data(), &option_index)) != -1) {
        switch (c) {
            case 'r':
            {
                //std::string role("");
                //value = optarg;
                /*
                if(role.compare("client") && (role.compare("server")) && (role.compare("both"))) {
                    std::cout << "Invalid value for --role, possible value is client, server or both "<< std::endl;
                    return(-1);
                }*/
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("role", value));
            }
            break;
            case 'h':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("peer-host", value));
            }
            break;
            case 'p':
            {
                auto tmp = std::stoi(optarg);
                value = *reinterpret_cast<std::uint16_t*>(&tmp);
                m_arguments.emplace_back(std::make_pair("peer-port", value));
            }
            break;
            case 'l':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("local-host", value));
            }
            break;
            case 'o':
            {
                auto tmp = std::stoi(optarg);
                value = *reinterpret_cast<std::uint16_t*>(&tmp);
                m_arguments.emplace_back(std::make_pair("local-port", value));
            }
            break;
            case 't':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("protocol", value));
            }
            break;
            case 'u':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("userid", value));
            }
            break;
            case 'a':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("password", value));
            }
            break;
            case 'm':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("timeout", value));
            }
            break;
            case 'n':
            {
                value = std::string(optarg);
                m_arguments.emplace_back(std::make_pair("long-poll", value));
            }
            break;
            default :
            {
                /// @error Option to be added.
            }
            break;
        }
    }
}

CommandLineArgument::~CommandLineArgument() {

}

bool CommandLineArgument::getValue(const std::string& argument, Value& value) {
    auto it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& ent) -> bool {
        if(ent.first == argument) {
            value = ent.second;
            return(true);
        }
        return(false);
    });

    return(it != m_arguments.end());
}














#endif /*__command_line_argument_cpp__*/