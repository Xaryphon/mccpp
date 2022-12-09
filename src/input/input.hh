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

struct input_data {
    input_data(const struct input_def &);

    std::string_view name;
    uint32_t raw[2];

    bool is_axis;
    union {
        float axis;
        struct {
            bool pressed;
            bool changed;
        } button;
    } value;
};

class iterator {
public:
    iterator(uint16_t value)
    : m_current(value)
    {}

    iterator &operator++()
    {
        ++m_current;
        return *this;
    }

    bool operator!=(const iterator &other)
    {
        return m_current != other.m_current;
    }

    input_data operator*();
private:
    uint16_t m_current;
};

struct iterable {
    iterator begin();
    iterator end();
};

namespace manager {
    void reset_deltas();
    void handle_event(SDL_Event &event);

    uint16_t get_input_count();
    input_data get_input_data(uint16_t idx);

    inline iterable get_inputs() { return {}; }
}

}
