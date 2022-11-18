#include "application.hh"

#include <chrono>
#include <SDL.h>

#include "logger.hh"
#include "renderer.hh"

namespace mccpp::application {

struct {
    bool should_quit = false;
} g_app = {};

// This a is pretty genius way of doing this if I do say so myshelf ;)
[[nodiscard]]
static bool _init_or_quit(bool init)
{
    if (!init)
        goto quit;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        MCCPP_F("SDL_Init failed: {}", SDL_GetError());
        goto sdl_init_failed;
    }

    if (!renderer::init()) {
        goto renderer_init_failed;
    }

    return true;

quit:
renderer_init_failed:
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
sdl_init_failed:
    return false;
}

[[nodiscard]]
static bool init()
{
    return _init_or_quit(true);
}

static void quit()
{
    (void) _init_or_quit(false);
}

static void poll_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
            g_app.should_quit = true;
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                g_app.should_quit = true;
                break;
            default:
                break;
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

    if (!init())
        return 1;

    unsigned frame_count = 0;
    unsigned last_frame_count = 0;
    auto fps_counter_next = std::chrono::steady_clock::now() + std::chrono::seconds(5);

    while (!g_app.should_quit) {
        auto now = std::chrono::steady_clock::now();
        if (now > fps_counter_next) {
            auto fps_counter_delta_time = std::chrono::duration_cast<std::chrono::duration<double>>(now - (fps_counter_next - std::chrono::seconds(5)));
            auto fps_counter_delta_frames = frame_count - last_frame_count;
            MCCPP_T("Average FPS over last 5s: {:.1f} ({} / {:.1f})", fps_counter_delta_frames / fps_counter_delta_time.count(), fps_counter_delta_frames, fps_counter_delta_time.count());
            fps_counter_next = now + std::chrono::seconds(5);
            last_frame_count = frame_count;
        }

        poll_events();
        renderer::render();

        frame_count++;
    }

    quit();
    return 0;
}

}
