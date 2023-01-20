#pragma once

#include <stdexcept>

namespace mccpp::proto {

class protocol_error : public std::runtime_error {
public:
    protocol_error(const std::string &what_arg)
    : std::runtime_error(what_arg)
    {}

    protocol_error(const char *what_arg)
    : std::runtime_error(what_arg)
    {}
};

class decode_error : public protocol_error {
public:
    decode_error(const std::string &what_arg)
    : protocol_error(what_arg)
    {}

    decode_error(const char *what_arg)
    : protocol_error(what_arg)
    {}
};

class encode_error : public protocol_error {
public:
    encode_error(const std::string &what_arg)
    : protocol_error(what_arg)
    {}

    encode_error(const char *what_arg)
    : protocol_error(what_arg)
    {}
};

}
