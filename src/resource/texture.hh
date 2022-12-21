#pragma once

#include <cassert>

#include "resource.hh"

namespace mccpp::resource {

class texture_object final : public object<texture_object> {
public:
    texture_object(std::string &&path)
    : object<texture_object>(std::move(path))
    {}

    uint32_t width() {
        assert(loaded());
        return m_width;
    }

    uint32_t height() {
        assert(loaded());
        return m_height;
    }

    const utility::runtime_array<std::byte> &pixels() {
        assert(loaded());
        return m_pixels;
    }

protected:
    bool do_load(bool force_reload) override;

private:
    uint32_t m_width;
    uint32_t m_height;
    utility::runtime_array<std::byte> m_pixels;
};

using texture = handle<texture_object>;

}
