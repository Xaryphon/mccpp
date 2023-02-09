#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <map>

#include <SDL.h>

#include "../application.hh"

namespace mccpp::input {

class input {
public:
    //std::string_view name() { return m_name; }

    float raw() { return m_raw; }
    float raw_old() { return m_old; }
    float raw_delta() { return raw() - raw_old(); }

    float value() { return clamp(raw(), -1.f, 1.f); }
    float old() { return clamp(raw(), -1.f, 1.f); }
    float delta() { return value() - old(); }

    bool pressed() { return raw() >= 0.5f; }
    bool was_pressed() { return raw_old() >= 0.5f; }
    bool down() { return !was_pressed() && pressed(); }
    bool up() { return was_pressed() && !pressed(); }

private:
    inline static float clamp(float value, float min, float max) {
        return value < min ? min : value > max ? max : value;
    }

    //std::string m_name;
    float m_raw = 0.f;
    float m_old = 0.f;

    friend class manager_impl;
};

using input_ref = input *;

class manager {
public:
    static std::unique_ptr<manager> create(application &);

    virtual ~manager() = default;

    virtual input_ref get(std::string_view) = 0;
    virtual void pre_events() = 0;
    virtual void handle_event(SDL_Event &) = 0;
    virtual void post_events() = 0;

    virtual void bind_mouse_x(input_ref neg, input_ref pos) = 0;
    virtual void bind_mouse_y(input_ref neg, input_ref pos) = 0;
    virtual void bind_mouse_button(uint8_t, input_ref) = 0;
    virtual void bind_keyboard(SDL_Scancode, input_ref) = 0;

    using iterator = std::map<std::string_view, input>::iterator;
    virtual iterator begin() = 0;
    virtual iterator end() = 0;
};

}
