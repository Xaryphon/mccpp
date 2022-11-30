#pragma once

#include <SDL.h>
#include <cstdint>
#include <string_view>

namespace mccpp::input {

struct axis {
    axis(std::string_view name);
    float value() const;

    const uint16_t idx;
};

struct button {
    button(std::string_view name);
    bool pressed() const;
    bool down() const;
    bool up() const;

    const uint16_t idx;
};

namespace mouse {
    void assign(axis x, axis y);

    float &sensitivity();
}

namespace keyboard {
    void assign(button, SDL_Scancode);
    void assign(axis, SDL_Scancode, SDL_Scancode);
}

namespace manager {
    void reset_deltas();
    void handle_event(SDL_Event &event);
}

}
