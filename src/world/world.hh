#pragma once

#include "chunk.hh"

namespace mccpp::world {

class world {
public:
    world(size_t world_height)
    : m_chunks(world_height / 16)
    {}

    chunk_manager &chunks() { return m_chunks; }

private:
    chunk_manager m_chunks;
};

}
