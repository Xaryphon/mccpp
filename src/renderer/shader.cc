#include "shader.hh"

#include "../utility/scope_guard.hh"

namespace mccpp::renderer {

shader::shader(std::initializer_list<std::pair<stage, std::string_view>> stages)
: m_id(0)
{
    m_shaders.reserve(stages.size());
    for (auto [stage, path] : stages) {
        m_shaders.emplace_back(stage, std::string(path));
    }
}

shader::~shader() {
    unload();
}

#define STAGE_TO_GL() \
    _STAGE_TO_GL(VERTEX, GL_VERTEX_SHADER) \
    _STAGE_TO_GL(TESS_CONTROL, GL_TESS_CONTROL_SHADER) \
    _STAGE_TO_GL(TESS_EVALUATION, GL_TESS_EVALUATION_SHADER) \
    _STAGE_TO_GL(GEOMETRY, GL_GEOMETRY_SHADER) \
    _STAGE_TO_GL(FRAGMENT, GL_FRAGMENT_SHADER) \
    _STAGE_TO_GL(COMPUTE, GL_COMPUTE_SHADER) \

static GLenum stage_to_gl(shader::stage stage) {
    switch (stage) {
#define _STAGE_TO_GL(from, to) \
    case shader::from: return to;
    STAGE_TO_GL()
#undef _STAGE_TO_GL
    };
    // unreachable
    abort();
}

static const char *stage_to_str(shader::stage stage) {
    switch (stage) {
#define _STAGE_TO_GL(from, to) \
    case shader::from: return #from;
    STAGE_TO_GL()
#undef _STAGE_TO_GL
    };
    // unreachable
    abort();
}

#undef STAGE_TO_GL

static GLuint compile_and_attach_shader(GLuint program, shader::stage stage, resource::shader &resource)
{
    if (!resource->load()) {
        return 0;
    }

    const char *source_pointer = resource->data().data();
    const int source_length = resource->data().length();

    GLuint shader = glCreateShader(stage_to_gl(stage));
    if (shader == 0) {
        MCCPP_E("glCreateShader failed for {}", stage_to_str(stage));
        return 0;
    }

    glShaderSource(shader, 1, &source_pointer, &source_length);
    glCompileShader(shader);

    GLint glstatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &glstatus);
    if (glstatus != GL_TRUE)
    {
        GLint elen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &elen);

        auto emsg = std::make_unique<char[]>(elen);
        glGetShaderInfoLog(shader, elen, &elen, emsg.get());
        MCCPP_E("glCompileShader failed for {} {}: {}",
                stage_to_str(stage), resource->resource_path(), emsg.get());
        glDeleteShader(shader);
        return 0;
    }

    glAttachShader(program, shader);
    glDeleteShader(shader);

    return shader;
}

static bool link_program(GLuint program)
{
    glLinkProgram(program);

    GLint glstatus;
    glGetProgramiv(program, GL_LINK_STATUS, &glstatus);
    if (glstatus != GL_TRUE)
    {
        GLint elen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &elen);

        auto emsg = std::make_unique<char[]>(elen);
        glGetProgramInfoLog(program, elen, &elen, emsg.get());
        MCCPP_E("glLinkProgram failed: {}", emsg.get());
        return false;
    }

    return true;
}

bool shader::load(bool force_reload) {
    (void)force_reload;

    if (!m_id) {
        m_id = glCreateProgram();
        if (!m_id) {
            MCCPP_E("glCreateProgram failed");
            return false;
        }
    }

    std::vector<GLuint> objects {};
    objects.reserve(m_shaders.size());

    for (auto &[stage, resource] : m_shaders) {
        GLuint shader = compile_and_attach_shader(m_id, stage, resource);
        if (!shader) {
            for (GLuint shader : objects) {
                glDetachShader(m_id, shader);
            }
            return false;
        }
        objects.emplace_back(shader);
    }

    bool link_success = link_program(m_id);
    for (GLuint shader : objects) {
        glDetachShader(m_id, shader);
    }
    return link_success;
}

void shader::unload() {
    if (m_id) {
        glDeleteShader(m_id);
        m_id = 0;
    }
}

}
