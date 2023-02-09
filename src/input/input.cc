#include "input.hh"

#include <array>
#include <bit>
#include <cassert>
#include <unordered_map>
#include <vector>

#include "../application.hh"
#include "../logger.hh"

namespace mccpp::input {

class manager_impl final : public manager {
public:
    manager_impl(application &) {}

    input_ref get(std::string_view) override;

    void pre_events() override;
    void handle_event(SDL_Event &) override;
    void post_events() override;

    void bind_mouse_x(input_ref neg, input_ref pos) override;
    void bind_mouse_y(input_ref neg, input_ref pos) override;
    void bind_mouse_button(uint8_t, input_ref) override;
    void bind_keyboard(SDL_Scancode, input_ref) override;

    iterator begin() override { return m_inputs.begin(); }
    iterator end() override { return m_inputs.end(); }

private:
    std::map<std::string_view, input> m_inputs;

    bool m_moved_mouse;
    float m_mouse_x_accumulation = 0.f;
    float m_mouse_y_accumulation = 0.f;
    input_ref m_mouse_x_n = {};
    input_ref m_mouse_x_p = {};
    input_ref m_mouse_y_n = {};
    input_ref m_mouse_y_p = {};

    std::array<input_ref, 5> m_mouse_buttons = {};

    std::array<input_ref, SDL_NUM_SCANCODES> m_keyboard = {};
};

std::unique_ptr<manager> manager::create(application &app) {
    return std::make_unique<manager_impl>(app);
}

input_ref manager_impl::get(std::string_view name) {
    return &m_inputs[name];
}

void manager_impl::pre_events() {
    if (m_moved_mouse) {
        if (m_mouse_x_n) {
            m_mouse_x_n->m_raw = 0.0f;
        }
        if (m_mouse_x_p) {
            m_mouse_x_p->m_raw = 0.0f;
        }
        if (m_mouse_y_n) {
            m_mouse_y_n->m_raw = 0.0f;
        }
        if (m_mouse_y_p) {
            m_mouse_y_p->m_raw = 0.0f;
        }

        m_mouse_x_accumulation = 0.0f;
        m_mouse_y_accumulation = 0.0f;
    }
    m_moved_mouse = false;
    for (auto &[key, value] : m_inputs) {
        (void)key;
        value.m_old = value.m_raw;
    }
}

void manager_impl::post_events() {
    if (m_moved_mouse) {
        if (m_mouse_x_n) {
            m_mouse_x_n->m_raw = m_mouse_x_accumulation < 0.0f ? -m_mouse_x_accumulation : 0.0f;
        }
        if (m_mouse_x_p) {
            m_mouse_x_p->m_raw = m_mouse_x_accumulation > 0.0f ?  m_mouse_x_accumulation : 0.0f;
        }
        if (m_mouse_y_n) {
            m_mouse_y_n->m_raw = m_mouse_y_accumulation < 0.0f ? -m_mouse_y_accumulation : 0.0f;
        }
        if (m_mouse_y_p) {
            m_mouse_y_p->m_raw = m_mouse_y_accumulation > 0.0f ?  m_mouse_y_accumulation : 0.0f;
        }
    }
}

void manager_impl::handle_event(SDL_Event &event) {
    switch (event.type) {
    case SDL_MOUSEMOTION:
        m_moved_mouse = true;
        m_mouse_x_accumulation += event.motion.xrel;
        m_mouse_y_accumulation -= event.motion.yrel;
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        assert(event.key.keysym.scancode < SDL_NUM_SCANCODES);
        if (!event.key.repeat) {
            bool down = event.type == SDL_KEYDOWN;
            if (input *ref = m_keyboard[event.key.keysym.scancode]) {
                ref->m_raw = down ? 1.0f : 0.0f;
            }
        }
        break;
    default: break;
    }
}

void manager_impl::bind_mouse_x(input_ref neg, input_ref pos) {
    m_mouse_x_n = neg;
    m_mouse_x_p = pos;
}

void manager_impl::bind_mouse_y(input_ref neg, input_ref pos) {
    m_mouse_y_n = neg;
    m_mouse_y_p = pos;
}

void manager_impl::bind_mouse_button(uint8_t button, input_ref ref) {
    assert(button > 0 && button < 6);
    m_mouse_buttons[button - 1] = ref;
}

void manager_impl::bind_keyboard(SDL_Scancode code, input_ref ref) {
    assert(code < SDL_NUM_SCANCODES);
    m_keyboard[code] = ref;
}

}
