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

#include "logger.hh"
#include "PerlinNoise.hpp"

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

struct block {
    bool is_air = true;
    glm::vec3 color;
};

struct chunk {
    std::array<block, 16 * 16 * 16> blocks;

    bool is_air_at(int x, int y, int z) const;
    inline bool is_air_at(glm::ivec3 pos) const
    {
        return is_air_at(pos.x, pos.y, pos.z);
    }

    std::tuple<std::vector<vertex>, std::vector<unsigned>> generate_vertices() const;
};

struct renderer {
    SDL_Window   *window = nullptr;
    SDL_GLContext gl_context = nullptr;

    GLuint shader;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    struct {
        glm::vec3 position;
        glm::vec3 rotation;
    } camera;
} g_self = {};

chunk generate_debug_chunk()
{
    chunk c;

    const siv::PerlinNoise::seed_type seed = 123456u;
    const siv::PerlinNoise perlin { seed };
    const double scale = 0.01;

    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            int h = perlin.octave2D_01(x * scale, z * scale, 4) * 16.0f;
            for (int y = 0; y < h; y++) {
                block &b = c.blocks[z * 256 + x * 16 + y];
                b.is_air = false;
                b.color = { 1.f, 1.f, 1.f };
            }
        }
    }

    return c;
}

chunk generate_debug_chunk2()
{
    chunk c;

    block &b = c.blocks[0];
    b.is_air = false;
    b.color = { 1.f, 1.f, 1.f };

    return c;
}

bool chunk::is_air_at(int x, int y, int z) const
{
    if (x < 0 || y < 0 || z < 0 || x >= 16 || y >= 16 || z >= 16)
        return true;
    else
        return blocks[z * 256 + x * 16 + y].is_air;
}

// glm::cross has a pointless assert for floating point only
static glm::ivec3 ivec3_cross(glm::ivec3 x, glm::ivec3 y)
{
    return {
		x.y * y.z - y.y * x.z,
		x.z * y.x - y.z * x.x,
		x.x * y.y - y.x * x.y,
    };
}

// NOTE: We don't care about winding order or about uv coordinates currently
void generate_face(std::vector<vertex> &vertices, std::vector<unsigned> &indicies,
                   block block, glm::vec3 position, glm::ivec3 normal)
{
    glm::ivec3 x = ivec3_cross(normal, { 1, 0, 0 });
    glm::ivec3 y = ivec3_cross(normal, { 0, 1, 0 });
    glm::ivec3 z = ivec3_cross(normal, { 0, 0, 1 });

    //MCCPP_T("n = ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d})",
    //        normal.x, normal.y, normal.z, x.x, x.y, x.z, y.x, y.y, y.z, z.x, z.y, z.z);
    glm::ivec3 p0 = x + y + z;
    glm::ivec3 p1 = ivec3_cross(normal, p0);
    auto p = std::to_array({
        p0,
        p1,
        -p0,
        -p1,
    });

    //MCCPP_T("    ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d})",
    //        p[0].x, p[0].y, p[0].z, p[1].x, p[1].y, p[1].z, p[2].x, p[2].y, p[2].z, p[3].x, p[3].y, p[3].z);

    size_t offset = vertices.size();
    indicies.emplace_back(offset + 0);
    indicies.emplace_back(offset + 1);
    indicies.emplace_back(offset + 3);
    indicies.emplace_back(offset + 1);
    indicies.emplace_back(offset + 2);
    indicies.emplace_back(offset + 3);

    // 03
    // 12

    (void)block;
    glm::vec3 fnormal = normal;

    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[0]) * 0.5f, /* block.color */ fnormal * 0.5f + 0.5f);
    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[1]) * 0.5f, /* block.color */ fnormal * 0.5f + 0.5f);
    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[2]) * 0.5f, /* block.color */ fnormal * 0.5f + 0.5f);
    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[3]) * 0.5f, /* block.color */ fnormal * 0.5f + 0.5f);
}

std::tuple<std::vector<vertex>, std::vector<unsigned>> chunk::generate_vertices() const
{
    constexpr std::array<glm::ivec3, 6> faces = {{
            {  1,  0,  0 },
            { -1,  0,  0 },
            {  0,  1,  0 },
            {  0, -1,  0 },
            {  0,  0,  1 },
            {  0,  0, -1 },
        }};
    std::vector<vertex> vertices;
    std::vector<unsigned> indicies;

    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                glm::ivec3 position = { x, y, z };
                const block &block = blocks[z * 256 + x * 16 + y];
                if (block.is_air)
                    continue;

                for (glm::ivec3 face : faces) {
                    if (is_air_at(position + face)) {
                        generate_face(vertices, indicies, block, position, face);
                    }
                }
            }
        }
    }

    return { vertices, indicies };
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(offsetof(vertex, color)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

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

    static const chunk chunk = generate_debug_chunk();
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
