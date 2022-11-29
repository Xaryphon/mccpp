#include "input.hh"

namespace mccpp::input {

axis::axis(std::string_view name)
{
    // TODO: Implement axis::axis(std::string_view name)
    (void) name;
}

float axis::value() const
{
    // TODO: Implement float axis::value()
    return 0;
}

button::button(std::string_view name)
{
    // TODO: Implement button::button(std::string_view name)
    (void) name;
}

bool button::pressed() const
{
    // TODO: Implement bool button::pressed()
    return false;
}

void manager::handle_event(SDL_Event &event)
{
    // TODO: Implement void manager::handle_event(SDL_Event &event)
    (void)event;
}

}
