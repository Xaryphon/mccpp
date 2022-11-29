#pragma once

#include <memory>
#include <string_view>

#include "resource.hh"

namespace mccpp::resource {

class texture : public resource {
public:
    texture() = default;
    texture(const char *path);
    ~texture() override;

    void load(const char *path);

    uint32_t width();
    uint32_t height();

    void *data();
    size_t length();

private:
    uint32_t m_width;
    uint32_t m_height;
    std::unique_ptr<char[]> m_data;
    size_t m_length;
};

}
