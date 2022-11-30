#pragma once

#include <SDL.h>
#include <cstdint>
#include <string_view>

namespace mccpp::input {

struct axis {
    explicit axis(std::string_view name);
    float value() const;

    operator float() const { return value(); }

    const uint16_t idx;
};

struct button {
    explicit button(std::string_view name);
    bool pressed() const;
    bool down() const;
    bool up() const;

    operator bool() const { return pressed(); }

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

    uint16_t get_input_count();
    std::string_view get_input_name(uint16_t idx);
    float get_input_value(uint16_t idx);
}

}
