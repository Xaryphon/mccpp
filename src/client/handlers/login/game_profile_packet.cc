#include "generated/client/handlers.hh"

#include "../../../logger.hh"
#include "../../../proto/serverbound/packets.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Login_Success
template<>
void client::handle_packet<proto::generated::clientbound::login::game_profile_packet>(proto::packet_reader &s) {
    using namespace proto::generated;

    class uuid uuid = s.read_uuid();
    std::string name = s.read_string<16>();
    int32_t prop_count = s.read_varint();
    MCCPP_I("Login success as \"{}\" ({}) with {} properties", name, uuid, prop_count);
    while (prop_count-- > 0) {
        std::string name = s.read_string<32767>();
        std::string value = s.read_string<32767>();
        bool is_signed = s.read_bool();
        std::string signature = is_signed ? s.read_string<32767>() : "";
        MCCPP_D("Property \"{}\": \"{}\" sig \"{}\"", name, value, signature);
    }

    m_state = connection_state::PLAY;
}

}
