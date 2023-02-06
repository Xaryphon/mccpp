#pragma once

#include <memory>

#include "application.hh"

namespace mccpp {

class game {
public:
    virtual ~game() = default;

    static std::unique_ptr<game> create(application &);

    virtual void on_frame() = 0;
    virtual float delta_time() = 0;
};

}
