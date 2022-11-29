#include "manager.hh"

#include <fstream>

namespace mccpp::resource::manager {

utility::runtime_array<char> read_file(const char *path)
{
    std::filebuf *filebuf;
    std::ifstream stream;
    stream.open(path, std::ios_base::in | std::ios_base::binary);
    filebuf = stream.rdbuf();

    size_t size = filebuf->pubseekoff(0, std::ios::end, std::ios::in);
    filebuf->pubseekpos(0, std::ios::in);

    utility::runtime_array<char> buffer { size };
    filebuf->sgetn(buffer.data(), buffer.size());

    stream.close();
    return buffer;
}

};
