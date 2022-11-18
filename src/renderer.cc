#include "renderer.hh"

#include <array>
#include <vector>
#include <fstream>
#include <memory>

#include <SDL.h>
#include <glad/glad.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4

#include "logger.hh"

namespace mccpp::renderer {

struct renderer {
    SDL_Window   *window = nullptr;
    SDL_GLContext gl_context = nullptr;

    GLuint shader;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
} g_self = {};

std::string _read_file(const std::string_view &path) {
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(& buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

static bool _compile_shader(GLuint shader, const std::string_view &path)
{
    std::string source = _read_file(path);
    const char *source_pointer = source.data();
    const int source_length = source.length();

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
        MCCPP_E("glCompileShader failed for {}: {}", path, emsg.get());
        return false;
    }

    return true;
}

static bool _link_program(GLuint program)
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

struct vertex {
    constexpr vertex(glm::vec3 position, glm::vec3 color)
    : position(position), color(color)
    {}

    constexpr vertex(float x, float y, float z, float r, float g, float b)
    : position(x, y, z), color(r, g, b)
    {}

    constexpr vertex(glm::vec3 position, float r, float g, float b)
    : position(position), color(r, g, b)
    {}

    constexpr vertex(float x, float y, float z, glm::vec3 color)
    : position(x, y, z), color(color)
    {}

    glm::vec3 position;
    glm::vec3 color;
};

[[nodiscard]]
static bool _init_or_destroy(bool init)
{
    if (!init)
        goto destroy;

    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);

    g_self.window = SDL_CreateWindow("mccpp",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!g_self.window) {
        MCCPP_F("SDL_CreateWindow failed: {}", SDL_GetError());
        goto create_window_failed;
    }

    g_self.gl_context = SDL_GL_CreateContext(g_self.window);
    if (!g_self.gl_context) {
        MCCPP_F("SDL_GL_CreateContext failed: {}", SDL_GetError());
        goto create_gl_context_failed;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        MCCPP_F("gladLoadGLLoader failed!");
        goto load_glad_failed;
    }

    SDL_GL_SetSwapInterval(1); // vsync

    MCCPP_I("Loaded GL {}.{}", GLVersion.major, GLVersion.minor);

    // FIXME: Handle errors with gl

    {
        unsigned vert = glCreateShader(GL_VERTEX_SHADER);
        _compile_shader(vert, "shaders/basic.vert");
        unsigned frag = glCreateShader(GL_FRAGMENT_SHADER);
        _compile_shader(frag, "shaders/basic.frag");

        g_self.shader = glCreateProgram();
        glAttachShader(g_self.shader, vert);
        glAttachShader(g_self.shader, frag);
        _link_program(g_self.shader);

        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    glGenVertexArrays(1, &g_self.VAO);
    glGenBuffers(1, &g_self.VBO);
    glGenBuffers(1, &g_self.EBO);

    glBindVertexArray(g_self.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_self.VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_self.EBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(offsetof(vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(offsetof(vertex, color)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    SDL_ShowWindow(g_self.window);

    return true;

destroy:
load_glad_failed:
    SDL_GL_DeleteContext(g_self.gl_context);
create_gl_context_failed:
    SDL_DestroyWindow(g_self.window);
create_window_failed:
    return false;
}

[[nodiscard]]
bool init()
{
    return _init_or_destroy(true);
}

void destroy()
{
    (void)_init_or_destroy(false);
}

void render()
{
    int width, height;
    SDL_GL_GetDrawableSize(g_self.window, &width, &height);

    constexpr vertex vertices[] = {
        { -.5f, -.5f,  .0f,  1.f, 0.f, 0.f },
        {  .5f, -.5f,  .0f,  0.f, 1.f, 0.f },
        {  .0f,  .5f,  .0f,  0.f, 0.f, 1.f },
    };

    constexpr unsigned indicies[] = {
        0, 1, 2,
    };

    glUseProgram(g_self.shader);
    glBindVertexArray(g_self.VAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_DYNAMIC_DRAW);

    glViewport(0, 0, width, height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, sizeof(indicies) / sizeof(*indicies), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(g_self.window);
}

}
