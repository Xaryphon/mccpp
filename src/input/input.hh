#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include <SDL.h>

#include "../application.hh"

namespace mccpp::input {

class manager;
class manager_impl;

class axis {
public:
    float value() const;

    operator float() const { return value(); }

private:
    axis(manager_impl &mgr, uint16_t idx)
    : m_manager(&mgr)
    , m_idx(idx)
    {}

    manager_impl *m_manager;
    uint16_t m_idx;

    friend class manager_impl;
};

class button {
public:
    bool pressed() const;
    bool down() const;
    bool up() const;

    operator bool() const { return pressed(); }

private:
    button(manager_impl &mgr, uint16_t idx)
    : m_manager(&mgr)
    , m_idx(idx)
    {}

    manager_impl *m_manager;
    uint16_t m_idx;

    friend class manager_impl;
};

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
    iterator(manager_impl &mgr, uint16_t value)
    : m_manager(mgr)
    , m_current(value)
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
    manager_impl &m_manager;
    uint16_t m_current;
};

class manager {
public:
    static std::unique_ptr<manager> create(application &);

    virtual ~manager() = default;

    virtual axis create_axis(std::string_view name) = 0;
    virtual button create_button(std::string_view name) = 0;

    virtual void mouse_assign(axis x, axis y) = 0;
    virtual float &mouse_sensitivity() = 0;

    virtual void keyboard_assign(button, SDL_Scancode) = 0;
    virtual void keyboard_assign(axis, SDL_Scancode, SDL_Scancode) = 0;

    virtual void reset_deltas() = 0;
    virtual void handle_event(SDL_Event &event) = 0;

    virtual iterator begin() = 0;
    virtual iterator end() = 0;
};

}
