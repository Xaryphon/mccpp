#pragma once

#include <glm/glm.hpp>

struct vertex {
    constexpr vertex(glm::vec3 position, glm::vec3 normal, glm::vec3 color, glm::vec2 uv)
    : position(position), normal(normal), color(color), uv(uv)
    {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
};
