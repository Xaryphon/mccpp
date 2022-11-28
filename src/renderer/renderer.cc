#include "renderer.hh"

#include <array>
#include <fstream>
#include <memory>
#include <vector>

#include <glad/glad.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <SDL.h>
#include <spng.h>

#include "../logger.hh"
#include "../PerlinNoise.hpp"
#include "../world/chunk.hh"
#include "vertex.hh"

namespace mccpp::renderer {

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

struct renderer {
    SDL_Window   *window = nullptr;
    SDL_GLContext gl_context = nullptr;

    GLuint shader;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    GLuint tex_uv;
    std::unique_ptr<char[]> tex_uv_data;
    size_t tex_uv_length;
    uint32_t tex_uv_width;
    uint32_t tex_uv_height;

    struct {
        glm::vec3 position;
        glm::vec3 rotation;
    } camera;
} g_self = {};

world::chunk generate_debug_chunk()
{
    world::chunk c;

    const siv::PerlinNoise::seed_type seed = 123456u;
    const siv::PerlinNoise perlin { seed };
    const double scale = 0.01;

    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            int h = perlin.octave2D_01(x * scale, z * scale, 4) * 16.0f;
            for (int y = 0; y < h; y++) {
                world::block &b = c.blocks[z * 256 + x * 16 + y];
                b.is_air = false;
                b.color = { 1.f, 1.f, 1.f };
            }
        }
    }

    return c;
}

world::chunk generate_debug_chunk2()
{
    world::chunk c;

    world::block &b = c.blocks[0];
    b.is_air = false;
    b.color = { 1.f, 1.f, 1.f };

    return c;
}

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
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
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

    if (!ImGui_ImplSDL2_InitForOpenGL(g_self.window, g_self.gl_context))
    {
        MCCPP_F("ImGui_ImplSDL2_InitForOpenGL failed!");
        goto init_imgui_sdl_failed;
    }

    if (!ImGui_ImplOpenGL3_Init(nullptr))
    {
        MCCPP_F("ImGui_ImplOpenGL3_Init failed!");
        goto init_imgui_gl_failed;
    }

    {
        // FIXME: Handle spng errors
        std::string png = _read_file("assets/dev/textures/misc/uv_16x16.png");
        spng_ctx *ctx = spng_ctx_new(0);
        assert(ctx);
        int error = spng_set_png_buffer(ctx, png.data(), png.size());
        assert(error == 0);
        spng_ihdr ihdr;
        error = spng_get_ihdr(ctx, &ihdr);
        assert(error == 0);
        size_t texture_size;
        error = spng_decoded_image_size(ctx, SPNG_FMT_RGB8, &texture_size);
        assert(error == 0);
        auto texture_data = std::make_unique<char[]>(texture_size);
        error = spng_decode_image(ctx, texture_data.get(), texture_size, SPNG_FMT_RGB8, 0);
        assert(error == 0);
        spng_ctx_free(ctx);

        g_self.tex_uv_data = std::move(texture_data);
        g_self.tex_uv_length = texture_size;
        g_self.tex_uv_width = ihdr.width;
        g_self.tex_uv_height = ihdr.height;
    }

    glGenTextures(1, &g_self.tex_uv);
    glBindTexture(GL_TEXTURE_2D, g_self.tex_uv);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_self.tex_uv_width, g_self.tex_uv_height, 0, GL_RGB, GL_UNSIGNED_BYTE, g_self.tex_uv_data.get());

    // FIXME: Handle errors with gl
    glEnable(GL_DEPTH_TEST);

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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(offsetof(vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(offsetof(vertex, color)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(offsetof(vertex, uv)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SDL_ShowWindow(g_self.window);

    return true;

destroy:
    ImGui_ImplOpenGL3_Shutdown();
init_imgui_gl_failed:
    ImGui_ImplSDL2_Shutdown();
init_imgui_sdl_failed:
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

void frame_start()
{
    SDL_GL_MakeCurrent(g_self.window, g_self.gl_context);
    ImGui_ImplOpenGL3_NewFrame();
}

void frame_end()
{
    int width, height;
    SDL_GL_GetDrawableSize(g_self.window, &width, &height);

    static const auto chunk = generate_debug_chunk();
    static auto [vertices, indicies] = chunk.generate_vertices();

    glm::vec3 &camera_position = g_self.camera.position;
    glm::vec3 &camera_rotation = g_self.camera.rotation;

    glm::mat4 V = glm::mat4(1.0f);
    V = glm::rotate(V, -camera_rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
    V = glm::rotate(V, camera_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
    V = glm::rotate(V, camera_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    V = glm::translate(V, camera_position * glm::vec3(-1.f, -1.f, -1.f));
    glm::mat4 P = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 VP = P * V;

    glUseProgram(g_self.shader);
    glUniformMatrix4fv(0, 1, false, glm::value_ptr(VP));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_self.tex_uv);
    glUniform1i(1, 0);

    glBindVertexArray(g_self.VAO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned), indicies.data(), GL_DYNAMIC_DRAW);

    glViewport(0, 0, width, height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(g_self.window);
}

glm::vec3 &camera::position()
{
    return g_self.camera.position;
}

glm::vec3 &camera::rotation()
{
    return g_self.camera.rotation;
}

}
