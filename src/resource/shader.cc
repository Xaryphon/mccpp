#include "shader.hh"

#include "../utility/runtime_array.hh"
#include "manager.hh"

namespace mccpp::resource {

shader::shader(const char *path)
{
    load(path);
}

shader::~shader()
{
}

void shader::load(const char *path)
{
    m_data = manager::read_file(path);
}

}
