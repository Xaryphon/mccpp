#include "application.hh"

#include <chrono>

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>

#include "game.hh"
#include "input/input.hh"
#include "logger.hh"
#include "renderer/renderer.hh"
#include "utility/misc.hh"
#include "utility/scope_guard.hh"

namespace mccpp {

#define MCCPP_SDL_FLAGS SDL_INIT_VIDEO

class application_impl final : public application {
public:
    renderer::renderer &renderer() override {
        assert(m_renderer.get());
        return *m_renderer;
    };

    class game &game() override {
        assert(m_game.get());
        return *m_game;
    };

    bool capture_mouse() override { return m_capture_mouse; }
    void capture_mouse(bool) override;

    bool should_exit() override { return m_should_exit; }
    void should_exit(bool) override;

private:
    bool m_capture_mouse;
    bool m_should_exit;

    unsigned m_frame_count;

    std::unique_ptr<renderer::renderer> m_renderer;
    std::unique_ptr<class game> m_game;

    friend int main(int argc, char **argv);
    friend unsigned application::frame_count();
};

static application_impl *g_app = nullptr;

unsigned application::frame_count() {
    return g_app->m_frame_count;
}

void application_impl::capture_mouse(bool state) {
    m_capture_mouse = state;
    SDL_SetRelativeMouseMode(state ? SDL_TRUE : SDL_FALSE);
}

void application_impl::should_exit(bool value) {
    m_should_exit = value;
}

static void poll_events(application_impl &app) {
    ImGuiIO &io = ImGui::GetIO();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            app.should_exit(true);
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                app.should_exit(true);
                break;
            default:
                break;
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (io.WantCaptureKeyboard)
                break;
            input::manager::handle_event(event);
            break;
        case SDL_MOUSEMOTION:
            if (app.capture_mouse()) {
                input::manager::handle_event(event);
            }
            break;
        default:
            break;
        }
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (SDL_InitSubSystem(MCCPP_SDL_FLAGS)) {
        MCCPP_E("SDL_InitSubSystem failed: {}", SDL_GetError());
        throw init_error("SDL_InitSubSystem");
    }
    MCCPP_SCOPE_EXIT {
        SDL_QuitSubSystem(MCCPP_SDL_FLAGS);
    };

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    MCCPP_SCOPE_EXIT {
        ImGui::DestroyContext();
    };
    ImGui::StyleColorsDark();

    application_impl app = {};
    g_app = &app;
    MCCPP_SCOPE_EXIT { g_app = nullptr; };

    app.m_renderer = renderer::renderer::create(app);
    app.m_game = game::create(app);

    input::button unlock_cursor { "unlock_cursor" };
    input::keyboard::assign(unlock_cursor, SDL_SCANCODE_LALT);
    app.capture_mouse(false);

    while (!app.should_exit()) {
        input::manager::reset_deltas();
        poll_events(app);

        if (unlock_cursor.down()) {
            app.capture_mouse(!app.capture_mouse());
        }

        app.m_renderer->start_frame();
        ImGui_ImplSDL2_NewFrame();
        if (app.capture_mouse()) {
            ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
        ImGui::NewFrame();

        app.m_game->on_frame();

        app.m_renderer->end_frame();

        app.m_frame_count++;
    }

    return 0;
}

}
