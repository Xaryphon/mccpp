#include "logger.hh"

#include <algorithm>
#include <cassert>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

namespace mccpp::logger {

std::string thread_default_name() {
    std::stringstream stream;
    stream << std::showbase << std::hex << std::this_thread::get_id();
    return stream.str();
}

thread_local std::string t_name = thread_default_name();

inline void _log_stderr(const Source &source, const Message &message)
{
    auto &stream = std::clog;

    char level_char;
    std::string_view color;
    switch (message.level) {
        case Level::FATAL: level_char = 'F'; color = "\033[91;1m"; break;
        case Level::ERROR: level_char = 'E'; color = "\033[91m";   break;
        case Level::WARN:  level_char = 'W'; color = "\033[93m";   break;
        case Level::INFO:  level_char = 'I'; color = "\033[97m";   break;
        case Level::DEBUG: level_char = 'D'; color = "\033[95m";   break;
        case Level::TRACE: level_char = 'T'; color = "\033[90m";   break;
    }

    stream << color;
    std::string_view content = message.data;
    while (!content.empty()) {
        auto line_end = content.find('\n');
        std::string_view line = content.substr(0, line_end);
        stream << level_char;
        stream << ' ' << t_name;
        stream << ' ' << source.file << ':' << source.line;
        stream << ' ' << line;
        if (line_end == content.npos) {
            break;
        }
        content = content.substr(line_end + 1, content.size() - line_end - 1);
        stream << '\n';
    }
    stream << "\033[0m";
    stream << '\n';

    stream.flush();
}

std::vector<Callback *> g_callbacks;

Callback::Callback() {
    g_callbacks.emplace_back(this);
}

Callback::~Callback() {
    auto iter = std::find(g_callbacks.begin(), g_callbacks.end(), this);
    assert(iter != g_callbacks.end());
    g_callbacks.erase(iter);
}

void set_thread_name(std::string &&name) {
    t_name = name;
}

void _message(const Source &source, const Message &message)
{
    _log_stderr(source, message);
    for (auto callback : g_callbacks) {
        callback->message(source, message);
    }
}

}
