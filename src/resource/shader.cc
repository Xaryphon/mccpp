#include "shader.hh"

namespace mccpp::resource {

bool shader_object::do_load(bool force_reload) {
    (void)force_reload;

    m_data = read_file(resource_path());
    return true;
}

}
