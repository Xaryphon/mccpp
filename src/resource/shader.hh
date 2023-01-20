#pragma once

#include <cassert>

#include "resource.hh"

namespace mccpp::resource {

class shader_object final : public object<shader_object> {
public:
    shader_object(std::string &&path)
    : object<shader_object>(std::move(path))
    {}

    std::string_view data() {
        assert(loaded());
        return { reinterpret_cast<char*>(m_data.data()), m_data.size() };
    }

protected:
    bool do_load(bool force_reload) override;

private:
    runtime_array<std::byte> m_data;
};

using shader = handle<shader_object>;

}
