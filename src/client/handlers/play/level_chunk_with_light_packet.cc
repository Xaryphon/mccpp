#include "../../handlers.hh"

#include "../../../logger.hh"
#include "../../../proto/serverbound/packets.hh"
#include "../../../proto/exceptions.hh"
#include "../../../utility/format.hh"
#include "../../../nbt.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Chunk_Data_and_Update_Light
template<>
void client::handle_packet<proto::generated::clientbound::play::level_chunk_with_light_packet>(proto::packet_reader &s) {
    int32_t chunk_x = s.read_i32();
    int32_t chunk_y = s.read_i32();
    nbt::nbt heightmaps { s };
    int32_t data_size = s.read_varint();
    if (data_size < 0) {
        throw proto::decode_error("invalid data size");
    }
    proto::packet_reader chunk_reader([&s]() -> std::byte {
        return s.read_byte();
    }, data_size);

    world::chunk_column &chunk_column = m_game.world().chunks().get(chunk_x, chunk_y);
    // https://wiki.vg/index.php?title=Chunk_Format&oldid=17949#Data_structure
    for (world::chunk &chunk : chunk_column) {
        chunk.load(chunk_reader);
    }

    chunk_reader.discard(chunk_reader.remaining());

    int32_t number_of_block_entities = s.read_varint();
    for (int32_t i = 0; i < number_of_block_entities; i++) {
        uint8_t packed_xz = s.read_u8();
        int16_t y = s.read_i16();
        int32_t type = s.read_varint();
        nbt::nbt data { s };
        (void)packed_xz;
        (void)y;
        (void)type;
    }
    bool trust_edges = s.read_bool();
    s.discard(s.remaining());
    //MCCPP_T("chunk {}, {}  block entities {}  trust edges {}", chunk_x, chunk_y, number_of_block_entities, trust_edges);
    (void)chunk_x;
    (void)chunk_y;
    (void)trust_edges;
}

}
