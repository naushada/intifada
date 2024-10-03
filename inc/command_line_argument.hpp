#ifndef __command_line_argument_hpp__
#define __command_line_argument_hpp__

#include <vector>
#include <variant>
#include <string>
#include <algorithm>
#include <iostream>

extern "C" {
    #include <getopt.h>
}

class Value: public std::variant<std::string, std::uint16_t, std::uint32_t, std::int32_t, std::nullptr_t> {
    public:
        /// @brief inherit the ctor, dtor of variant into Value
        using variant::variant;
        enum VType {VSTRING = 0, VUINT16 = 1, VUINT32 = 2, VINT32 = 3, VNull = 4};

        bool getString(std::string& out) {
            if(this->index() == VType::VSTRING) {
                out = std::get<std::string>(*this);
                return(true);
            }
            return(false);
        }

        bool getUint16(std::uint16_t& out) {
            if(this->index() == VType::VUINT16) {
                out = std::get<std::uint16_t>(*this);
                return(true);
            }
            return(false);
        }

        bool getUint32(std::uint32_t& out) {
            if(this->index() == VType::VUINT32) {
                out = std::get<std::uint32_t>(*this);
                return(true);
            }
            return(false);
        }

        bool getInt32(std::int32_t& out) {
            if(this->index() == VType::VINT32) {
                out = std::get<std::int32_t>(*this);
                return(true);
            }
            return(false);
        }

        bool isNull() {
            if(this->index() == VType::VNull) {
                return(true);
            }
            return(false);
        }
};

class CommandLineArgument {
    public:
        CommandLineArgument(std::int32_t argc, char* argv[]);
        bool parseOptions(std::int32_t argc, char* argv[]);
        ~CommandLineArgument();
        bool getValue(const std::string& argument, Value& value);
        
    private:
        std::vector<std::pair<std::string, Value>> m_arguments;

};




















#endif /*__command_line_argument_hpp__*/