#include "renderer.hh"

#include <array>
#include <fstream>
#include <memory>
#include <optional>
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
#include "../resource/shader.hh"
#include "../resource/texture.hh"
#include "../utility/misc.hh"
#include "../utility/scope_guard.hh"
#include "../world/chunk.hh"
#include "../cvar.hh"
#include "shader.hh"
#include "vertex.hh"

namespace mccpp::renderer {

class renderer_impl final : public renderer {
public:
    renderer_impl(application &);
    ~renderer_impl();

    void start_frame() override;
    void end_frame() override;

    struct camera &camera() override {
        return m_camera;
    }

private:
    SDL_Window   *m_window = nullptr;
    SDL_GLContext m_gl_context = nullptr;

    class shader m_shader {
        { shader::VERTEX,   "mccpp/shaders/basic.vert" },
        { shader::FRAGMENT, "mccpp/shaders/basic.frag" },
    };
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_EBO = 0;

    GLuint m_tex_uv = 0;
    resource::texture m_res_uv { "mccpp/textures/misc/uv_16x16.png" };

    struct camera m_camera = {};
};

std::unique_ptr<renderer> renderer::create(application &app) {
    return std::make_unique<renderer_impl>(app);
}

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

renderer_impl::renderer_impl(application &app) {
    (void)app;

    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);

    m_window = SDL_CreateWindow("mccpp",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window) {
        MCCPP_F("SDL_CreateWindow failed: {}", SDL_GetError());
        throw init_error("SDL_CreateWindow");
    }
    MCCPP_SCOPE_FAIL { SDL_DestroyWindow(m_window); };

    m_gl_context = SDL_GL_CreateContext(m_window);
    if (!m_gl_context) {
        MCCPP_F("SDL_GL_CreateContext failed: {}", SDL_GetError());
        throw init_error("SDL_GL_CreateContext");
    }
    MCCPP_SCOPE_FAIL { SDL_GL_DeleteContext(m_gl_context); };

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        MCCPP_F("gladLoadGLLoader failed!");
        throw init_error("gladLoadGLLoader");
    }

    SDL_GL_SetSwapInterval(1); // vsync

    MCCPP_I("Loaded GL {}.{}", GLVersion.major, GLVersion.minor);

    if (!ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context))
    {
        MCCPP_F("ImGui_ImplSDL2_InitForOpenGL failed!");
        throw init_error("ImGui_ImplSDL2_InitForOpenGL");
    }
    MCCPP_SCOPE_FAIL { ImGui_ImplSDL2_Shutdown(); };

    if (!ImGui_ImplOpenGL3_Init(nullptr))
    {
        MCCPP_F("ImGui_ImplOpenGL3_Init failed!");
        throw init_error("ImGui_ImplOpenGL3_Init");
    }
    MCCPP_SCOPE_FAIL { ImGui_ImplOpenGL3_Shutdown(); };

    m_shader.load();
    m_res_uv->load();

    glGenTextures(1, &m_tex_uv);
    glBindTexture(GL_TEXTURE_2D, m_tex_uv);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_res_uv->width(), m_res_uv->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, m_res_uv->pixels().data());

    // FIXME: Handle errors with gl
    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

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

    cvar::manager &cvar_manager = app.cvar_manager();

    cvar_manager.create("r_cull_faces", 1, [](float value) {
        if (value == 0.f) {
            glDisable(GL_CULL_FACE);
        } else if (value == 1.f) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else if (value == 2.f) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } else if (value == 3.f) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
        } else {
            return false;
        }
        return true;
    });

    cvar_manager.create("r_wireframe", 0, [](float value) {
        if (value == 0.f) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else if (value == 1.f) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else if (value == 2.f) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        } else {
            return false;
        }
        return true;
    });

    SDL_ShowWindow(m_window);
}

renderer_impl::~renderer_impl()
{
    m_shader.unload();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_window);
}

void renderer_impl::start_frame()
{
    SDL_GL_MakeCurrent(m_window, m_gl_context);
    ImGui_ImplOpenGL3_NewFrame();
}

void renderer_impl::end_frame()
{
    int width, height;
    SDL_GL_GetDrawableSize(m_window, &width, &height);

    static const auto chunk = generate_debug_chunk();
    static auto [vertices, indicies] = chunk.generate_vertices();

    glm::vec3 &camera_position = m_camera.position;
    glm::vec3 &camera_rotation = m_camera.rotation;

    glm::mat4 V = glm::mat4(1.0f);
    V = glm::rotate(V, -camera_rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
    V = glm::rotate(V, camera_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
    V = glm::rotate(V, camera_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    V = glm::translate(V, camera_position * glm::vec3(-1.f, -1.f, -1.f));
    glm::mat4 P = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 VP = P * V;

    glUseProgram(m_shader.id());
    glUniformMatrix4fv(0, 1, false, glm::value_ptr(VP));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex_uv);
    glUniform1i(1, 0);

    glBindVertexArray(m_VAO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned), indicies.data(), GL_DYNAMIC_DRAW);

    glViewport(0, 0, width, height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(m_window);
}

}
