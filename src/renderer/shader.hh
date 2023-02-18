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

    shader();
    ~shader();

    bool load(resource::manager &, std::initializer_list<std::pair<stage, std::string_view>>);
    void unload();

    bool loaded() {
        return m_id;
    }

    GLuint id() {
        return m_id;
    }

private:
    GLuint m_id;
};

}
