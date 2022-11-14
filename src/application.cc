#include "application.hh"

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_video.h>

#include "logger.hh"

namespace mccpp::application {

struct {
    SDL_Window   *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    bool should_quit = false;
} g_app = {};

// This a is pretty genius way of doing this if I do say so myshelf ;)
[[nodiscard]]
inline bool _init_or_quit(bool init)
{
    if (!init)
        goto quit;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        MCCPP_F("SDL_Init failed: {}", SDL_GetError());
        goto sdl_init_failed;
    }

    g_app.window = SDL_CreateWindow("mccpp",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!g_app.window) {
        MCCPP_F("SDL_CreateWindow failed: {}", SDL_GetError());
        goto create_window_failed;
    }

    // FIXME: Use OpenGL instead
    g_app.renderer = SDL_CreateRenderer(g_app.window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!g_app.renderer) {
        MCCPP_F("SDL_CreateRenderer failed: {}", SDL_GetError());
        goto create_renderer_failed;
    }

    SDL_ShowWindow(g_app.window);
    return true;

quit:
create_renderer_failed:
    SDL_DestroyWindow(g_app.window);
create_window_failed:
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
sdl_init_failed:
    return false;
}

[[nodiscard]]
inline bool init()
{
    return _init_or_quit(true);
}

inline void quit()
{
    (void) _init_or_quit(false);
}

inline void poll_events()
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
            }
            break;
        }
    }
}

inline void render()
{
    SDL_SetRenderDrawColor(g_app.renderer, 64, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(g_app.renderer);
    SDL_RenderPresent(g_app.renderer);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (!init())
        return 1;

    while (!g_app.should_quit) {
        poll_events();
        render();
    }

    quit();
    return 0;
}

}
