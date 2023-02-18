#include "shader.hh"

namespace mccpp::resource {

shader_object::shader_object(manager &mgr, const identifier &id, load_flags flags) {
    (void)flags;
    m_data = mgr.read_file("{}/shaders/{}", id.name_space(), id.name());
}

}
