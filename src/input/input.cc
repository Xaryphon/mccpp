#include "input.hh"

#include <array>
#include <bit>
#include <cassert>
#include <vector>

#include "../application.hh"
#include "../logger.hh"

namespace mccpp::input {

struct input_def {
    const std::string name;
    // axis:
    //     bool  is_axis :  1 = true
    //           unused  : 31
    //     float value   : 32
    // button:
    //     bool  is_axis :  1 = false
    //            unused : 30
    //     bool  pressed :  1
    //     u32   set_on  : 32
    uint32_t data[2];

    input_def(std::string_view name, bool is_axis)
    : name(name), data{(uint32_t)is_axis << 31, 0}
    {}

    input_def(const input_def &) = delete;
    input_def(input_def &&) = default;

    bool is_axis() const
    {
        return data[0] >> 31;
    }

    float get_axis() const
    {
        static_assert(sizeof(uint32_t) == sizeof(float));
        assert(is_axis());
        return std::bit_cast<float>(data[1]);
    }

    void set_axis(float value)
    {
        static_assert(sizeof(uint32_t) == sizeof(float));
        assert(is_axis());
        data[1] = std::bit_cast<uint32_t>(value);
    }

    void add_axis(float value)
    {
        static_assert(sizeof(uint32_t) == sizeof(float));
        assert(is_axis());
        float old_value = std::bit_cast<float>(data[1]);
        data[1] = std::bit_cast<uint32_t>(old_value + value);
    }

    bool get_button_pressed() const
    {
        assert(!is_axis());
        return data[0] & 1;
    }

    bool get_button_changed() const
    {
        return data[1] == application::frame_count();
    }

    bool get_button_down() const
    {
        return get_button_pressed() && get_button_changed();
    }

    bool get_button_up() const
    {
        return !get_button_pressed() && get_button_changed();
    }

    void set_button(bool value)
    {
        assert(!is_axis());
        if (get_button_pressed() == value)
            return;
        if (value)
            data[0] |=  1;
        else
            data[0] &= ~1;
        data[1] = application::frame_count();
    }
};

class input_ref {
public:
    static const uint16_t MAX_INPUTS = 0x7fff;

    input_ref()
    : m_dir(0), m_idx(0x7fff)
    {}

    input_ref(uint16_t idx, bool direction = false)
    : m_dir(direction), m_idx(idx)
    {}

    bool valid() {
        return m_idx != 0x7fff;
    }

    bool direction() {
        assert(valid());
        return m_dir;
    }

    uint16_t idx() {
        assert(valid());
        return m_idx;
    }

private:
    uint16_t m_dir :  1;
    uint16_t m_idx : 15;
};

class manager_impl final : public manager {
public:
    axis create_axis(std::string_view name) override;
    button create_button(std::string_view name) override;

    void mouse_assign(axis x, axis y) override;
    float &mouse_sensitivity() override;

    void keyboard_assign(button, SDL_Scancode) override;
    void keyboard_assign(axis, SDL_Scancode, SDL_Scancode) override;

    void reset_deltas() override;
    void handle_event(SDL_Event &event) override;

    iterator begin() override { return { *this, 0 }; }
    iterator end() override { return { *this, static_cast<uint16_t>(m_inputs.size()) }; }

private:
    uint16_t find_or_new(std::string_view name, bool is_axis);

    std::vector<input_def> m_inputs;

    struct {
        input_ref x = {};
        input_ref y = {};

        float sensitivity = 1.f;
    } m_mouse;

    std::array<input_ref, SDL_NUM_SCANCODES> m_keyboard;

    friend class axis;
    friend class button;
    friend class iterator;
};

std::unique_ptr<manager> manager::create(application &) {
    return std::make_unique<manager_impl>();
}

uint16_t manager_impl::find_or_new(std::string_view name, bool is_axis) {
    input_def *inputs = m_inputs.data();
    size_t inputs_size = m_inputs.size();
    for (size_t i = 0; i < inputs_size; i++) {
        if (inputs[i].name == name)
            return i;
    }

    assert(inputs_size <= input_ref::MAX_INPUTS);
    m_inputs.emplace_back(name, is_axis);
    return inputs_size;
}

axis manager_impl::create_axis(std::string_view name) {
    return { *this, find_or_new(name, true) };
}

button manager_impl::create_button(std::string_view name) {
    return { *this, find_or_new(name, false) };
}

float axis::value() const
{
    return m_manager->m_inputs[m_idx].get_axis();
}

bool button::pressed() const
{
    return m_manager->m_inputs[m_idx].get_button_pressed();
}

bool button::down() const
{
    return m_manager->m_inputs[m_idx].get_button_down();
}

bool button::up() const
{
    return m_manager->m_inputs[m_idx].get_button_up();
}

void manager_impl::mouse_assign(axis x, axis y)
{
    m_mouse.x = { x.m_idx };
    m_mouse.y = { y.m_idx };
}

float &manager_impl::mouse_sensitivity()
{
    return m_mouse.sensitivity;
}

void manager_impl::keyboard_assign(button b, SDL_Scancode key)
{
    assert(key < SDL_NUM_SCANCODES);
    m_keyboard[key] = { b.m_idx };
}

void manager_impl::keyboard_assign(axis axis, SDL_Scancode pos, SDL_Scancode neg)
{
    assert(pos < SDL_NUM_SCANCODES);
    assert(neg < SDL_NUM_SCANCODES);
    m_keyboard[pos] = { axis.m_idx, false };
    m_keyboard[neg] = { axis.m_idx, true  };
}

input_data::input_data(const input_def &data)
: name(data.name)
, raw{data.data[0], data.data[1]}
, is_axis(data.is_axis())
{
    if (is_axis) {
        value.axis = data.get_axis();
    } else {
        value.button.pressed = data.get_button_pressed();
        value.button.changed = data.get_button_changed();
    }
}

input_data iterator::operator*() {
    return { m_manager.m_inputs[m_current] };
}

void manager_impl::reset_deltas()
{
    if (input_ref x = m_mouse.x; x.valid()) {
        assert(!x.direction());
        m_inputs[x.idx()].set_axis(0.f);
    }
    if (input_ref y = m_mouse.y; y.valid()) {
        assert(!y.direction());
        m_inputs[y.idx()].set_axis(0.f);
    }
}

void manager_impl::handle_event(SDL_Event &event)
{
    switch (event.type) {
    case SDL_MOUSEMOTION:
        if (input_ref x = m_mouse.x; x.valid()) {
            m_inputs[x.idx()].add_axis(m_mouse.sensitivity * event.motion.xrel);
        }
        if (input_ref y = m_mouse.y; y.valid()) {
            m_inputs[y.idx()].add_axis(m_mouse.sensitivity * -event.motion.yrel);
        }
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        assert(event.key.keysym.scancode < SDL_NUM_SCANCODES);
        if (!event.key.repeat) {
            bool down = event.type == SDL_KEYDOWN;
            if (input_ref ref = m_keyboard[event.key.keysym.scancode]; ref.valid()) {
                input_def &input = m_inputs[ref.idx()];
                if (input.is_axis()) {
                    bool direction = ref.direction();
                    float value = down ^ direction ? 1.f : -1.f;
                    input.add_axis(value);
                } else {
                    assert(!ref.direction());
                    input.set_button(down);
                }
            }
        }
        break;
    default: break;
    }
}

}
