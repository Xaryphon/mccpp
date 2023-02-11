#pragma once

#include <cstdint>
#include <fmt/format.h>

namespace mccpp {

class uuid {
public:
    constexpr uuid() : m_data{0, 0} {}
    constexpr uuid(uint64_t high, uint64_t low) : m_data{high, low} {}

    constexpr uint64_t high() const noexcept { return m_data[0]; }
    constexpr uint64_t low() const noexcept { return m_data[1]; }

    constexpr bool is_nil() const noexcept {
        return m_data[0] == 0 && m_data[1] == 0;
    }

    int operator==(const uuid &other) {
        return m_data[0] == other.m_data[0] && m_data[1] == other.m_data[1];
    }

private:
    uint64_t m_data[2];
};

}

template<>
struct fmt::formatter<mccpp::uuid> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
            throw format_error("invalid format");
        return it;
    }

  template <typename FormatContext>
  auto format(const mccpp::uuid &id, FormatContext &ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
        id.high() >> 32, id.high() >> 16 & 0xffff, id.high() & 0xffff,
        id.low() >> 48, id.low() & 0xffffffffffff);
  }
};
