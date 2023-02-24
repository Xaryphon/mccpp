#include "resource.hh"

#include <algorithm>
#include <cassert>
#include <fstream>

#include "../logger.hh"
#include "../utility/scope_guard.hh"

namespace mccpp::resource {

runtime_array<std::byte> manager::read_file(std::string_view path) {
    std::filebuf *filebuf;
    std::ifstream stream;
    std::string full_path = fmt::format("assets/{}", path);
    stream.open(full_path, std::ios_base::in | std::ios_base::binary);
    if (stream.fail()) {
        MCCPP_E("Failed to open file {}", path);
        return runtime_array<std::byte>();
    }
    filebuf = stream.rdbuf();

    auto size = filebuf->pubseekoff(0, std::ios::end, std::ios::in);
    if (size < 0) {
        MCCPP_E("Failed to seek to end of {}", path);
        return runtime_array<std::byte>();
    }
    filebuf->pubseekpos(0, std::ios::in);

    runtime_array<std::byte> buffer { (size_t)size };
    filebuf->sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());

    stream.close();
    return buffer;
}

resource *manager::get_internal(std::type_index type, const identifier &id,
                   load_flags flags, create_resource_fn factory)
{
    auto iter = m_resources.find({ type, id });
    if (iter != m_resources.end())
        return iter->second.get();

    if (std::any_of(m_init_list.begin(), m_init_list.end(), [&type, &id](const auto &item) {
            return item.first == type && item.second == id;
        }))
    {
        MCCPP_E("Detected a cycle for the following resources:");
        for (auto &item : m_init_list) {
            MCCPP_E("- {} (type: {})", item.second.full(), item.first.name());
        }
        throw std::runtime_error("resource cycle detected");
    }

    auto init_iter = m_init_list.emplace(m_init_list.begin(), type, id);
    MCCPP_SCOPE_EXIT { m_init_list.erase(init_iter); };

    std::unique_ptr<resource> res = (this->*factory)(id, flags);

    auto [iter2, inserted] = m_resources.try_emplace({ type, id }, std::move(res));
    assert(inserted);
    return iter2->second.get();
}

}
