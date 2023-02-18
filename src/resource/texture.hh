#pragma once

#include <cassert>

#include "resource.hh"

namespace mccpp::resource {

class texture_object final : public resource {
public:
    texture_object(manager &, const identifier &, load_flags);

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    const runtime_array<std::byte> &pixels() const { return m_pixels; }

private:
    uint32_t m_width;
    uint32_t m_height;
    runtime_array<std::byte> m_pixels;
};

using texture = handle<texture_object>;

}
