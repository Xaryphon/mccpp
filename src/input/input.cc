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

struct {
    std::vector<input_def> inputs;

    struct {
        input_ref x = {};
        input_ref y = {};

        float sensitivity = 1.f;
    } mouse;

    std::array<input_ref, SDL_NUM_SCANCODES> keyboard;
} g_self;

uint16_t find_or_new(std::string_view name, bool is_axis)
{
    input_def *inputs = g_self.inputs.data();
    size_t inputs_size = g_self.inputs.size();
    for (size_t i = 0; i < inputs_size; i++) {
        if (inputs[i].name == name)
            return i;
    }

    assert(inputs_size <= input_ref::MAX_INPUTS);
    g_self.inputs.emplace_back(name, is_axis);
    return inputs_size;
}

axis::axis(std::string_view name)
: idx(find_or_new(name, true))
{}

float axis::value() const
{
    return g_self.inputs[idx].get_axis();
}

button::button(std::string_view name)
: idx(find_or_new(name, false))
{}

bool button::pressed() const
{
    return g_self.inputs[idx].get_button_pressed();
}

bool button::down() const
{
    return g_self.inputs[idx].get_button_down();
}

bool button::up() const
{
    return g_self.inputs[idx].get_button_up();
}

void mouse::assign(axis x, axis y)
{
    g_self.mouse.x = { x.idx };
    g_self.mouse.y = { y.idx };
}

float &mouse::sensitivity()
{
    return g_self.mouse.sensitivity;
}

void keyboard::assign(button b, SDL_Scancode key)
{
    assert(key < SDL_NUM_SCANCODES);
    g_self.keyboard[key] = { b.idx };
}

void keyboard::assign(axis axis, SDL_Scancode pos, SDL_Scancode neg)
{
    assert(pos < SDL_NUM_SCANCODES);
    assert(neg < SDL_NUM_SCANCODES);
    g_self.keyboard[pos] = { axis.idx, false };
    g_self.keyboard[neg] = { axis.idx, true  };
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

input_data iterator::operator*()
{
    return { g_self.inputs[m_current] };
}

iterator iterable::begin()
{
    return { 0 };
}

iterator iterable::end()
{
    return { static_cast<uint16_t>(g_self.inputs.size()) };
}

void manager::reset_deltas()
{
    if (input_ref x = g_self.mouse.x; x.valid()) {
        assert(!x.direction());
        g_self.inputs[x.idx()].set_axis(0.f);
    }
    if (input_ref y = g_self.mouse.y; y.valid()) {
        assert(!y.direction());
        g_self.inputs[y.idx()].set_axis(0.f);
    }
}

void manager::handle_event(SDL_Event &event)
{
    switch (event.type) {
    case SDL_MOUSEMOTION:
        if (input_ref x = g_self.mouse.x; x.valid()) {
            g_self.inputs[x.idx()].add_axis(g_self.mouse.sensitivity * event.motion.xrel);
        }
        if (input_ref y = g_self.mouse.y; y.valid()) {
            g_self.inputs[y.idx()].add_axis(g_self.mouse.sensitivity * -event.motion.yrel);
        }
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        assert(event.key.keysym.scancode < SDL_NUM_SCANCODES);
        if (!event.key.repeat) {
            bool down = event.type == SDL_KEYDOWN;
            if (input_ref ref = g_self.keyboard[event.key.keysym.scancode]; ref.valid()) {
                input_def &input = g_self.inputs[ref.idx()];
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

uint16_t manager::get_input_count()
{
    return g_self.inputs.size();
}

input_data manager::get_input_data(uint16_t idx)
{
    return { g_self.inputs[idx] };
}

}
