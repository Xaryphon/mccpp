#include "resource.hh"

#include <fstream>

#include "../logger.hh"

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

    auto [iter2, inserted] = m_resources.try_emplace({ type, id }, (this->*factory)(id, flags));
    return iter2->second.get();
}

}
