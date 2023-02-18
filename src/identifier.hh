#pragma once

#include <string>
#include <string_view>

namespace mccpp {

class identifier {
public:
    constexpr identifier()
    : m_data()
    , m_separator_offset(0)
    {}

    constexpr identifier(const char *from)
    : identifier(std::string(from))
    {}

    constexpr identifier(std::string_view from)
    : identifier(std::string(from))
    {}

    constexpr identifier(std::string &&from)
    : identifier(std::move(from), "minecraft")
    {}

    constexpr identifier(std::string &&from, std::string_view default_name_space)
    : m_data(std::move(from))
    , m_separator_offset(m_data.find(':'))
    {
        if (m_separator_offset == std::string::npos) {
            m_separator_offset = default_name_space.size();
            m_data.insert(0, ":");
            m_data.insert(0, default_name_space);
        }
    }

    constexpr bool empty() const { return m_data.empty(); }
    constexpr std::string_view full() const { return m_data; }
    constexpr std::string_view name_space() const { return full().substr(0, m_separator_offset); }
    constexpr std::string_view name() const { return full().substr(m_separator_offset + 1); }

    bool operator==(const identifier &other) const {
        return  m_data == other.m_data;
    }

private:
    std::string m_data;
    size_t m_separator_offset;
};

}
