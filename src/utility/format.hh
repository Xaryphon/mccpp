#pragma once

#include <span>
#include <fmt/format.h>

template<>
struct fmt::formatter<std::span<const std::byte>> {
    bool show_chars = false;

    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it == 'c') {
            show_chars = true;
            ++it;
        }

        if (it != end && *it != '}')
            throw format_error("invalid format");

        return it;
    }

    template<typename FormatContext>
    auto format(const std::span<const std::byte> &span, FormatContext &ctx) const -> decltype(ctx.out()) {
        auto it = ctx.out();
        if (show_chars) {
            for (size_t l = 0; l < span.size(); l += 16) {
                if (l != 0)
                    it = fmt::format_to(it, "\n");

                size_t w = span.size() - l;
                if (w > 16) w = 16;
                for (size_t i = 0; i < w; i++) {
                    if (i != 0 && i % 4 == 0) {
                        it = fmt::format_to(it, " ");
                    }
                    it = fmt::format_to(it, "{:02x}", span[l + i]);
                }

                if (w < 16) {
                    size_t s = (16 * 2 + 3) - 2 * w - w / 4;
                    for (size_t i = 0; i < s; i++) {
                        it = fmt::format_to(it, " ");
                    }
                }

                it = fmt::format_to(it, " ");
                for (size_t i = 0; i < w; i++) {
                    std::byte byte = span[l + i];
                    if (isprint(static_cast<int>(byte))) {
                        it = fmt::format_to(it, "{}", static_cast<char>(byte));
                    } else {
                        it = fmt::format_to(it, ".");
                    }
                }
            }
        } else {
            for (size_t i = 0; i < span.size(); ++i) {
                if (i != 0 && i % 4 == 0) {
                    it = fmt::format_to(it, " ");
                }
                it = fmt::format_to(it, "{:02x}", span[i]);
            }
        }
        return it;
    }
};
