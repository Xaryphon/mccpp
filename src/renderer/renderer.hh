#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "../application.hh"

namespace mccpp::renderer {

struct camera {
    glm::vec3 position;
    glm::vec3 rotation;
};

class renderer {
public:
    static std::unique_ptr<renderer> create(application &);

    virtual void start_frame() = 0;
    virtual void end_frame() = 0;

    virtual struct camera &camera() = 0;
};

};
