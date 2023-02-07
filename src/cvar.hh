#pragma once

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <string_view>

#include "application.hh"

namespace mccpp::cvar {

class cvar {
public:
    using callback = std::function<bool(float&)>;

    cvar(std::string_view name, std::string_view help, callback cb, float value)
    : m_name(name)
    , m_help(help)
    , m_callback(cb)
    , m_value(value)
    {}

    const std::string &name() const {
        return m_name;
    }

    std::string_view help() const {
        return m_help;
    }

    float value() const {
        return m_value;
    }

    bool set_value(float v);

    auto operator<=>(const cvar &other) const {
        return m_name <=> other.m_name;
    }

private:
    const std::string m_name;
    std::string_view m_help;
    callback m_callback;
    float m_value;
};

class manager {
public:
    using storage = std::set<cvar>;

    struct iterator : storage::iterator {
        explicit iterator(storage::iterator &&from)
        : storage::iterator(from)
        {}
        cvar &operator*() {
            return const_cast<cvar &>(storage::iterator::operator*());
        }
    };

    static std::unique_ptr<manager> create(application &);

    cvar &create(std::string_view name,
                 float value,
                 std::string_view help,
                 cvar::callback callback);

    cvar &create(std::string_view name,
                 float value,
                 cvar::callback callback)
    {
        return create(name, value, "", callback);
    }

    iterator begin() { return iterator(m_cvars.begin()); }
    iterator end()   { return iterator(m_cvars.end());   }

private:
    storage m_cvars;
};

}
