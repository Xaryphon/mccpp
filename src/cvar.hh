#pragma once

#include <functional>
#include <set>
#include <string>
#include <string_view>

namespace mccpp {

class cvar {
public:
    using storage = std::set<cvar>;
    using callback = std::function<bool(float&)>;

    struct iterator : storage::iterator {
        explicit iterator(storage::iterator &&from)
        : storage::iterator(from)
        {}
        cvar &operator*() {
            return const_cast<cvar &>(storage::iterator::operator*());
        }
    };

    static cvar &create(std::string_view name,
                        float value,
                        std::string_view help,
                        callback callback);

    static cvar &create(std::string_view name,
                        float value,
                        callback callback)
    {
        return create(name, value, "", callback);
    }

    static iterator begin() { return iterator(s_cvars.begin()); }
    static iterator end()   { return iterator(s_cvars.end());   }

    struct range {
        static iterator begin() { return cvar::begin(); }
        static iterator end()   { return cvar::end();   }
    };

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
    static storage s_cvars;

    const std::string m_name;
    std::string_view m_help;
    callback m_callback;
    float m_value;
};

}
