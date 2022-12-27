#pragma once

#include <string_view>
#include <fmt/core.h>

namespace mccpp::logger {

struct Source {
    const char *func;
    const char *file;
    int line;
};

enum class Level {
    FATAL,
    ERROR,
    INFO,
    WARN,
    DEBUG,
    TRACE,
};

struct Message {
    std::string_view data;
    Level level;
};

class Callback {
public:
    Callback();
    virtual ~Callback();

    virtual void message(const Source &source, const Message &message) = 0;
};

void set_thread_name(std::string &&name);

void _message(const Source &source, const Message &message);

inline void _message(const Source &source, Level level, const std::string_view &message)
{
    _message(source, { message, level });
}

template<typename... Args>
void _message(const Source &source, Level level, fmt::format_string<Args...> format, const Args &...args)
{
    std::string message = fmt::vformat(format, fmt::make_format_args(args...));
    _message(source, { std::string_view(message), level });
}

}

#define _MCCPP_LOG(lvl, ...) mccpp::logger::_message( \
        { __func__, __FILE__, __LINE__ }, lvl, __VA_ARGS__)
#define MCCPP_F(...) _MCCPP_LOG(mccpp::logger::Level::FATAL, __VA_ARGS__)
#define MCCPP_E(...) _MCCPP_LOG(mccpp::logger::Level::ERROR, __VA_ARGS__)
#define MCCPP_I(...) _MCCPP_LOG(mccpp::logger::Level::INFO,  __VA_ARGS__)
#define MCCPP_W(...) _MCCPP_LOG(mccpp::logger::Level::WARN,  __VA_ARGS__)
#define MCCPP_D(...) _MCCPP_LOG(mccpp::logger::Level::DEBUG, __VA_ARGS__)
#define MCCPP_T(...) _MCCPP_LOG(mccpp::logger::Level::TRACE, __VA_ARGS__)
