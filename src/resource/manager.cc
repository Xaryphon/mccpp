#include "manager.hh"

#include <fstream>

#include "../logger.hh"

namespace mccpp::resource::manager {

utility::runtime_array<char> read_file(const char *path)
{
    std::filebuf *filebuf;
    std::ifstream stream;
    stream.open(path, std::ios_base::in | std::ios_base::binary);
    if (stream.fail()) {
        MCCPP_E("Failed to open file {}", path);
        return utility::runtime_array<char>();
    }
    filebuf = stream.rdbuf();

    auto size = filebuf->pubseekoff(0, std::ios::end, std::ios::in);
    if (size < 0) {
        MCCPP_E("Failed to seek to end of {}", path);
        return utility::runtime_array<char>();
    }
    filebuf->pubseekpos(0, std::ios::in);

    utility::runtime_array<char> buffer { (size_t)size };
    filebuf->sgetn(buffer.data(), buffer.size());

    stream.close();
    return buffer;
}

};
