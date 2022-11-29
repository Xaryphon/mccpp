#pragma once

#include <memory>
#include <string_view>

#include "resource.hh"
#include "../utility/runtime_array.hh"

namespace mccpp::resource {

class shader : public resource {
public:
    shader() = default;
    shader(const char *path);
    ~shader() override;

    void load(const char *path);

    char *data() { return m_data.data(); }
    size_t length() { return m_data.size(); }

private:
    utility::runtime_array<char> m_data;
};

}
