#include "generated/client/handlers.hh"

#include "../../../logger.hh"
#include "../../../proto/serverbound/packets.hh"
#include "../../../utility/format.hh"
#include "../../../nbt.hh"

namespace mccpp::client {

static std::vector<std::byte> string_to_bytes(std::string_view str) {
    std::vector<std::byte> vec {};
    vec.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        vec.emplace_back(std::byte(str[i]));
    }
    return vec;
}

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Login_.28play.29
template<>
void client::handle_packet<proto::generated::clientbound::play::login_packet>(proto::packet_reader &s) {
    using namespace proto::generated;

    int32_t entity_id = s.read_i32();
    bool is_hardcore = s.read_bool();
    uint8_t gamemode = s.read_u8();
    int8_t previous_gamemode = s.read_i8();
    int32_t dimension_count = s.read_varint();
    std::vector<std::string> dimension_names {};
    while (dimension_count-- > 0) {
        dimension_names.emplace_back(s.read_identifier());
    }
    nbt::nbt registry_codec(s);
    std::string dimension_type = s.read_identifier();
    std::string dimension_name = s.read_identifier();
    int64_t hashed_seed = s.read_i64();
    int32_t max_players = s.read_varint();
    int32_t view_distance = s.read_varint();
    int32_t simulation_distance = s.read_varint();
    bool reduced_debug_info = s.read_bool();
    bool enable_respawn_screen = s.read_bool();
    bool is_debug = s.read_bool();
    bool is_flat = s.read_bool();
    bool has_death_location = s.read_bool();
    std::string death_dimension_name = has_death_location ? s.read_identifier() : "";
    proto::position death_location = has_death_location ? s.read_position() : 0;

    MCCPP_D("Login (Play): Entity ID: {}  Hardcore: {}  Mode: {}  Previous Mode: {}", entity_id, is_hardcore, gamemode, previous_gamemode);
    MCCPP_D("Dimensions ({}):", dimension_names.size());
    for (const std::string &name : dimension_names) {
        MCCPP_D("- {}", name);
    }
    MCCPP_D("Dimension: {} ({})", dimension_name, dimension_type);
    MCCPP_D("Hashed seed: {}  Max Players: {}  View: {}  Sim: {}", hashed_seed, max_players, view_distance, simulation_distance);
    MCCPP_D("Reduced Debug: {}  Respawn Screen: {}  Is Debug: {}  Is Flat: {}  Has Death Location: {}", reduced_debug_info, enable_respawn_screen, is_debug, is_flat, has_death_location);
    if (has_death_location) {
        MCCPP_D("Death Location: {} {} {} {}", death_dimension_name, death_location.x(), death_location.y(), death_location.z());
    }

    // FIXME: get world height from registry_codec/dimension_type/height and handle the rest of the registry
    m_game.create_world(384);

    queue_send<serverbound::play::custom_payload_packet>({
        .channel = "minecraft:brand",
        .data = string_to_bytes("mccpp"),
    });

    queue_send<serverbound::play::client_information_packet>({
        .locale = "en_US",
        .view_distance = 10,
        .chat_mode = 2,
        .chat_colors = false,
        .displayed_skin_parts = 0x00,
        .main_hand = 1,
        .enable_text_filtering = false,
        .allow_server_listings = false,
    });
}

}
