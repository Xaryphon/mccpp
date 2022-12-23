#pragma once

#include <stdexcept>

namespace mccpp::proto {

class decode_exception : public std::runtime_error {
public:
    decode_exception(const std::string &what_arg)
    : std::runtime_error(what_arg)
    {}

    decode_exception(const char *what_arg)
    : std::runtime_error(what_arg)
    {}
};

}
