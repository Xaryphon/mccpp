#pragma once

#include <memory>

#include "application.hh"
#include "world/world.hh"

namespace mccpp {

class game {
public:
    virtual ~game() = default;

    static std::unique_ptr<game> create(application &);

    world::world &create_world(size_t height) {
        return m_world.emplace(height);
    }

    world::world &world() {
        return m_world.value();
    }

    virtual void on_frame() = 0;
    virtual float delta_time() = 0;

private:
    std::optional<world::world> m_world;
};

}
