#include "input.hh"

#include <array>
#include <cassert>
#include <vector>

#include "../application.hh"
#include "../logger.hh"

namespace mccpp::input {

const uint16_t IDX_INVALID = -1;

struct input_data {
    explicit input_data(std::string_view name)
    : name(name), value(0), last_update(0)
    {}

    float update(float new_value)
    {
        last_update = application::frame_count();
        return value = new_value;
    }

    const std::string name;
    float value;
    unsigned last_update;
};

struct input_reference {
    input_reference()
    : idx(IDX_INVALID)
    {}

    input_reference(uint16_t idx, float amplitude)
    : idx(idx), amplitude(amplitude)
    {}

    uint16_t idx;
    float amplitude;
};

struct {
    std::vector<input_data> inputs;

    struct {
        uint16_t x = IDX_INVALID;
        uint16_t y = IDX_INVALID;

        float sensitivity = 1.f;
    } mouse;

    std::array<input_reference, SDL_NUM_SCANCODES> keyboard;
} g_self;

uint16_t find_or_new(std::string_view name)
{
    input_data *inputs = g_self.inputs.data();
    size_t inputs_size = g_self.inputs.size();
    for (size_t i = 0; i < inputs_size; i++) {
        if (inputs[i].name == name)
            return i;
    }

    assert(inputs_size < IDX_INVALID);
    g_self.inputs.emplace_back(name);
    return inputs_size;
}

axis::axis(std::string_view name)
: idx(find_or_new(name))
{}

float axis::value() const
{
    return g_self.inputs[idx].value;
}

button::button(std::string_view name)
: idx(find_or_new(name))
{}

bool button::pressed() const
{
    return g_self.inputs[idx].value > 0.5f;
}

bool button::down() const
{
    input_data &input = g_self.inputs[idx];
    return input.value > 0.5f && input.last_update == application::frame_count();
}

bool button::up() const
{
    input_data &input = g_self.inputs[idx];
    return input.value < 0.5f && input.last_update == application::frame_count();
}

void mouse::assign(axis x, axis y)
{
    g_self.mouse.x = x.idx;
    g_self.mouse.y = y.idx;
}

float &mouse::sensitivity()
{
    return g_self.mouse.sensitivity;
}

void keyboard::assign(button b, SDL_Scancode key)
{
    assert(key < SDL_NUM_SCANCODES);
    g_self.keyboard[key] = { b.idx, 1.f };
}

void keyboard::assign(axis axis, SDL_Scancode pos, SDL_Scancode neg)
{
    assert(pos < SDL_NUM_SCANCODES);
    assert(neg < SDL_NUM_SCANCODES);
    g_self.keyboard[pos] = { axis.idx,  1.f };
    g_self.keyboard[neg] = { axis.idx, -1.f };
}

void manager::reset_deltas()
{
    if (uint16_t x = g_self.mouse.x; x != IDX_INVALID) {
        g_self.inputs[x].value = 0.f;
    }
    if (uint16_t y = g_self.mouse.y; y != IDX_INVALID) {
        g_self.inputs[y].value = 0.f;
    }
}

void manager::handle_event(SDL_Event &event)
{
    switch (event.type) {
    case SDL_MOUSEMOTION:
        if (uint16_t x = g_self.mouse.x; x != IDX_INVALID) {
            g_self.inputs[x].update(g_self.mouse.sensitivity * event.motion.xrel);
        }
        if (uint16_t y = g_self.mouse.y; y != IDX_INVALID) {
            g_self.inputs[y].update(g_self.mouse.sensitivity * -event.motion.yrel);
        }
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        assert(event.key.keysym.scancode < SDL_NUM_SCANCODES);
        if (!event.key.repeat) {
            bool down = event.type == SDL_KEYDOWN;
            if (input_reference &ref = g_self.keyboard[event.key.keysym.scancode]; ref.idx != IDX_INVALID) {
                g_self.inputs[ref.idx].update(down ? ref.amplitude : 0.f);
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

std::string_view manager::get_input_name(uint16_t idx)
{
    return g_self.inputs[idx].name;
}

float manager::get_input_value(uint16_t idx)
{
    return g_self.inputs[idx].value;
}

}
