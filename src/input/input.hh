#pragma once

#include <SDL_events.h>
#include <cstdint>
#include <string_view>

namespace mccpp::input {

class axis {
public:
    axis(std::string_view name);
    float value() const;

private:
    uint8_t m_idx;
};

class button {
public:
    button(std::string_view name);
    bool pressed() const;

private:
    uint8_t m_idx;
};

namespace manager {

void handle_event(SDL_Event &e);

}

}
