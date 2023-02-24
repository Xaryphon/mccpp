#pragma once

#include <array>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "../renderer/vertex.hh"
#include "../proto/packet.hh"

namespace mccpp::world {

struct block {
    bool is_air = true;
    glm::vec3 color;
};

void generate_face(std::vector<vertex> &vertices, std::vector<unsigned> &indicies,
                   block block, glm::vec3 position, glm::ivec3 normal);

struct chunk {
    std::array<block, 16 * 16 * 16> blocks;

    bool is_air_at(int x, int y, int z) const;
    inline bool is_air_at(glm::ivec3 pos) const
    {
        return is_air_at(pos.x, pos.y, pos.z);
    }

    void load(proto::packet_reader &);

    std::tuple<std::vector<vertex>, std::vector<unsigned>> generate_vertices() const;
};

class chunk_column {
public:
    using iterator = chunk *;

    chunk_column(size_t count)
    : m_chunks(std::make_unique<chunk[]>(count))
    {}

    chunk &operator[](size_t y) { return m_chunks[y]; }
    size_t count() { return m_count; }

    iterator begin() { return m_chunks.get(); }
    iterator end() { return m_chunks.get() + m_count; }

private:
    std::unique_ptr<chunk[]> m_chunks;
    size_t m_count;
};

class chunk_manager {
public:
    chunk_manager(size_t height_in_chunks)
    : m_height_in_chunks(height_in_chunks)
    {}

    chunk_column *try_get(int32_t x, int32_t y) {
        auto iter = m_columns.find(chunk_pos_to_idx(x, y));
        if (iter == m_columns.end())
            return nullptr;
        return &iter->second;
    }

    chunk_column &get(int32_t x, int32_t y) {
        if (auto column = try_get(x, y))
            return *column;
        auto iter = m_columns.emplace(chunk_pos_to_idx(x, y), m_height_in_chunks);
        assert(iter.second);
        return iter.first->second;
    }

    void unload(int32_t, int32_t);

    size_t height_in_chunks() {
        return m_height_in_chunks;
    }

private:
    constexpr uint64_t chunk_pos_to_idx(int32_t x, int32_t y) noexcept {
        return uint64_t(std::bit_cast<uint32_t>(x)) << 32 | std::bit_cast<uint32_t>(y);
    }

    size_t m_height_in_chunks;
    std::unordered_map<uint64_t, chunk_column> m_columns;
};

}
