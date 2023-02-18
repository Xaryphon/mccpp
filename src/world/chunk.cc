#include "chunk.hh"

#include "../utility/extract_bits.hh"

namespace mccpp::world {

bool chunk::is_air_at(int x, int y, int z) const
{
    if (x < 0 || y < 0 || z < 0 || x >= 16 || y >= 16 || z >= 16)
        return true;
    else
        return blocks[z * 256 + x * 16 + y].is_air;
}

static const block &block_from_global_palette(int32_t) {
    static block b { false, { 0, 0, 0 } };
    return b;
}

static void load_blocks(chunk &c, proto::packet_reader &s) {
    uint8_t bits_per_entry = s.read_u8();
    if (bits_per_entry == 0) {
        int32_t value = s.read_varint();
        const block &gpb = block_from_global_palette(value);
        for (block &b : c.blocks) {
            b = gpb;
        }
        /* data_array_length */ s.read_varint();
        return;
    }

    if (bits_per_entry <= 8) {
        if (bits_per_entry < 4) {
            bits_per_entry = 4;
        }

        int32_t palette_length = s.read_varint();
        if (palette_length < 0)
            throw proto::decode_error("invalid palette length");
        std::vector<int32_t> palette = {};
        palette.reserve(palette_length);
        while (palette_length-- > 0) {
            palette.emplace_back(s.read_varint());
        }

        /* data_array_length */ s.read_varint();
        std::vector<uint64_t> data_array {};
        // 4096 blocks * bits per entry = total bits
        // total bits / 64 = u64 count
        size_t u64_count = 4096 * bits_per_entry / 64;
        data_array.reserve(u64_count + 1);
        for (size_t i = 0; i < u64_count; i++) {
            data_array.emplace_back(s.read_u64());
        }
        // add an extra unused value to make parsing easier
        data_array.emplace_back(0);

        for (size_t i = 0; i < 4096; i++) {
            uint64_t entry = extract_bits(data_array, i * bits_per_entry, bits_per_entry);
            (void)entry;
        }

        return;
    }
}

static void load_biomes(chunk &c, proto::packet_reader &s) {
    (void)c;
    (void)s;
}

void chunk::load(proto::packet_reader &s) {
    /* block_count */ s.read_i16();
    load_blocks(*this, s);
    load_biomes(*this, s);
}

// glm::cross has a pointless assert for floating point only
static glm::ivec3 ivec3_cross(glm::ivec3 x, glm::ivec3 y)
{
    return {
		x.y * y.z - y.y * x.z,
		x.z * y.x - y.z * x.x,
		x.x * y.y - y.x * x.y,
    };
}

void generate_face(std::vector<vertex> &vertices, std::vector<unsigned> &indicies,
                   block block, glm::vec3 position, glm::ivec3 normal)
{
    assert((normal.x == 0) + (normal.y == 0) + (normal.z == 0) == 2);

    glm::ivec3 x = ivec3_cross(normal, { 1, 0, 0 });
    glm::ivec3 y = ivec3_cross(normal, { 0, 1, 0 });
    glm::ivec3 z = ivec3_cross(normal, { 0, 0, 1 });

    //MCCPP_T("n = ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d})",
    //        normal.x, normal.y, normal.z, x.x, x.y, x.z, y.x, y.y, y.z, z.x, z.y, z.z);
    glm::ivec3 p0 = x + y + z;
    glm::ivec3 p1 = ivec3_cross(normal, p0);
    auto p = std::to_array({
        p0,
        p1,
        -p0,
        -p1,
    });

    //MCCPP_T("    ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d}) ({: d}, {: d}, {: d})",
    //        p[0].x, p[0].y, p[0].z, p[1].x, p[1].y, p[1].z, p[2].x, p[2].y, p[2].z, p[3].x, p[3].y, p[3].z);

    size_t offset = vertices.size();
    indicies.emplace_back(offset + 0);
    indicies.emplace_back(offset + 1);
    indicies.emplace_back(offset + 3);
    indicies.emplace_back(offset + 1);
    indicies.emplace_back(offset + 2);
    indicies.emplace_back(offset + 3);

    // 03
    // 12

    (void)block;
    glm::vec3 fnormal = normal;

    // NOTE: We swap the UV vertically (because OpenGL texture bottom is 0.f)
    glm::vec2 uv0 = { 0.0f, 1.0f };
    glm::vec2 uv1 = { 1.0f, 1.0f };
    glm::vec2 uv2 = { 1.0f, 0.0f };
    glm::vec2 uv3 = { 0.0f, 0.0f };

    if (normal.x < 0 || normal.y < 0 || normal.z > 0) {
        auto tmp = uv3;
        uv3 = uv2;
        uv2 = uv1;
        uv1 = uv0;
        uv0 = tmp;
    } else if (normal.y > 0) {
        std::swap(uv3, uv1);
        std::swap(uv2, uv0);
    }

    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[0]) * 0.5f, fnormal, /* block.color */ fnormal * 0.5f + 0.5f, uv0);
    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[1]) * 0.5f, fnormal, /* block.color */ fnormal * 0.5f + 0.5f, uv1);
    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[2]) * 0.5f, fnormal, /* block.color */ fnormal * 0.5f + 0.5f, uv2);
    vertices.emplace_back(position + fnormal * 0.5f + static_cast<glm::vec3>(p[3]) * 0.5f, fnormal, /* block.color */ fnormal * 0.5f + 0.5f, uv3);
}

std::tuple<std::vector<vertex>, std::vector<unsigned>> chunk::generate_vertices() const
{
    constexpr std::array<glm::ivec3, 6> faces = {{
            {  1,  0,  0 },
            { -1,  0,  0 },
            {  0,  1,  0 },
            {  0, -1,  0 },
            {  0,  0,  1 },
            {  0,  0, -1 },
        }};
    std::vector<vertex> vertices;
    std::vector<unsigned> indicies;

    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                glm::ivec3 position = { x, y, z };
                const block &block = blocks[z * 256 + x * 16 + y];
                if (block.is_air)
                    continue;

                for (glm::ivec3 face : faces) {
                    if (is_air_at(position + face)) {
                        generate_face(vertices, indicies, block, position, face);
                    }
                }
            }
        }
    }

    return { vertices, indicies };
}

}
