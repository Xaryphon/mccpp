#pragma once

#include <array>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>

#include "../renderer/vertex.hh"

namespace mccpp::world {

struct block {
    bool is_air = true;
    glm::vec3 color;
};

struct chunk {
    std::array<block, 16 * 16 * 16> blocks;

    bool is_air_at(int x, int y, int z) const;
    inline bool is_air_at(glm::ivec3 pos) const
    {
        return is_air_at(pos.x, pos.y, pos.z);
    }

    std::tuple<std::vector<vertex>, std::vector<unsigned>> generate_vertices() const;
};

}
