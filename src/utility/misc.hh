#pragma once

#include <stdexcept>

#define MCCPP_FORWARD(to, ...) to(__VA_ARGS__)

namespace mccpp::utility {

class init_error : public std::runtime_error {
public:
    init_error(const std::string& what_arg)
    : std::runtime_error(what_arg)
    {}

    init_error(const char* what_arg)
    : std::runtime_error(what_arg)
    {}
};

}
