#pragma once

#include "vfs/vfs.hh"

namespace mccpp::resource {

struct block_state_variant {
    std::string match;
    uint8_t flags;

    float rotation_x() {
        return (flags & 0x3) * 90.f;
    }

    float rotation_y() {
        return (flags >> 2 & 0x3) * 90.f;
    }

    bool uvlock() {
        return flags & 0x10;
    }
};

class block_state {
public:
    block_state(vfs::vfs &, const vfs::tree_node &);

    auto begin() { return m_variants.begin(); }
    auto end() { return m_variants.end(); }

private:
    std::vector<block_state_variant> m_variants;
};

}
