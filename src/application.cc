#include "application.hh"

#include <chrono>

#include <asio.hpp>
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>

#include "client/client.hh"
#include "cvar.hh"
#include "game.hh"
#include "input/input.hh"
#include "logger.hh"
#include "renderer/renderer.hh"
#include "resource/resource.hh"
#include "utility/misc.hh"
#include "utility/scope_guard.hh"
#include "vfs/host.hh"
#include "vfs/vfs.hh"

namespace mccpp {

#define MCCPP_SDL_FLAGS SDL_INIT_VIDEO

class application_impl final : public application {
public:
    cvar::manager &cvar_manager() override {
        assert(m_cvar_manager.get());
        return *m_cvar_manager;
    };

    vfs::vfs &assets() override {
        assert(m_assets.get());
        return *m_assets;
    }

    resource::manager &resource_manager() override {
        assert(m_resource_manager.get());
        return *m_resource_manager;
    };

    input::manager &input_manager() override {
        assert(m_input_manager.get());
        return *m_input_manager;
    }

    renderer::renderer &renderer() override {
        assert(m_renderer.get());
        return *m_renderer;
    };

    class game &game() override {
        assert(m_game.get());
        return *m_game;
    };

    client::client &client() override {
        assert(m_client.get());
        return *m_client;
    };

    bool capture_mouse() override { return m_capture_mouse; }
    void capture_mouse(bool) override;

    bool should_exit() override { return m_should_exit; }
    void should_exit(bool) override;

private:
    bool m_capture_mouse;
    bool m_should_exit;

    unsigned m_frame_count;

    std::unique_ptr<cvar::manager> m_cvar_manager;
    std::unique_ptr<vfs::vfs> m_assets;
    std::unique_ptr<resource::manager> m_resource_manager;
    std::unique_ptr<input::manager> m_input_manager;
    std::unique_ptr<renderer::renderer> m_renderer;
    std::unique_ptr<class game> m_game;
    std::unique_ptr<client::client> m_client;

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
            app.input_manager().handle_event(event);
            break;
        case SDL_MOUSEMOTION:
            if (app.capture_mouse()) {
                app.input_manager().handle_event(event);
            }
            break;
        default:
            break;
        }
    }
}

int main(int argc, char **argv) {
    using startup_clock = std::chrono::steady_clock;
    startup_clock::time_point sc_start = startup_clock::now();

    logger::set_thread_name("main");

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

    asio::io_context io;

    application_impl app = {};
    g_app = &app;
    MCCPP_SCOPE_EXIT { g_app = nullptr; };
    app.m_cvar_manager = cvar::manager::create(app);
    startup_clock::time_point sc_vfs_start = startup_clock::now();
    app.m_assets = std::make_unique<vfs::vfs>();
    app.m_assets->storages().emplace_back(std::make_unique<vfs::host_storage>("assets"));
    app.m_assets->build_tree();
    startup_clock::time_point sc_vfs_finish = startup_clock::now();
    auto sc_vfs_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(sc_vfs_finish - sc_vfs_start).count();
    MCCPP_I("Build vfs tree in {}.{:06} ms", sc_vfs_ns / 1'000'000, sc_vfs_ns % 1'000'000);
    startup_clock::time_point sc_res_start = startup_clock::now();
    app.m_resource_manager = std::make_unique<resource::manager>(app);
    app.m_resource_manager->load();
    startup_clock::time_point sc_res_finish = startup_clock::now();
    auto sc_res_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(sc_res_finish - sc_res_start).count();
    MCCPP_I("Loaded resources in {}.{:06} ms", sc_res_ns / 1'000'000, sc_res_ns % 1'000'000);
    app.m_input_manager = input::manager::create(app);
    app.m_renderer = renderer::renderer::create(app);
    app.m_game = game::create(app);
    app.m_client = std::make_unique<client::client>(app);

    //app.m_client->connect(io, "127.0.0.1", 25564);

    input::input_ref unlock_cursor = app.m_input_manager->get("unlock_cursor");
    app.m_input_manager->bind_keyboard(SDL_SCANCODE_LALT, unlock_cursor);
    app.capture_mouse(false);

    startup_clock::time_point sc_finish = startup_clock::now();
    auto sc_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(sc_finish - sc_start).count();

    MCCPP_I("Total {}.{:06} ms", sc_ns / 1'000'000, sc_ns % 1'000'000);

    while (!app.should_exit()) {
        io.poll();

        app.m_input_manager->pre_events();
        poll_events(app);
        app.m_input_manager->post_events();

        if (unlock_cursor->down()) {
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
