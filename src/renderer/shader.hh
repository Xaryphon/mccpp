#pragma once

#include <initializer_list>
#include <string_view>
#include <utility>
#include <vector>

#include <glad/glad.h>

#include "../resource/shader.hh"

namespace mccpp::renderer {

class shader {
public:
    enum stage {
        VERTEX,
        TESS_CONTROL,
        TESS_EVALUATION,
        GEOMETRY,
        FRAGMENT,
        COMPUTE,
    };

    shader(std::initializer_list<std::pair<stage, std::string_view>>);
    ~shader();

    bool load(bool force_reload = false);
    void unload();

    bool loaded() {
        return m_id;
    }

    GLuint id() {
        return m_id;
    }

private:
    std::vector<std::pair<stage, resource::shader>> m_shaders;
    GLuint m_id;
};

}
