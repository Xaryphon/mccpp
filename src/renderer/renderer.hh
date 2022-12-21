#pragma once

#include <glm/glm.hpp>

namespace mccpp::renderer {

void init();
void destroy();

void frame_start();
void frame_end();

namespace camera {

glm::vec3 &position();
glm::vec3 &rotation();

}

};
