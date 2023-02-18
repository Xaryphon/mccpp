#pragma once

#include <cassert>

#include "resource.hh"

namespace mccpp::resource {

class shader_object final : public resource {
public:
    shader_object(manager &, const identifier &, load_flags);

    std::string_view data() const {
        return { reinterpret_cast<char*>(m_data.data()), m_data.size() };
    }

private:
    runtime_array<std::byte> m_data;
};

using shader = handle<shader_object>;

}
