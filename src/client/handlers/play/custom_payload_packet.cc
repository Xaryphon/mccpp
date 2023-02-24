#include "generated/client/handlers.hh"

#include "../../../logger.hh"
#include "../../../proto/serverbound/packets.hh"
#include "../../../utility/format.hh"
#include "../../../nbt.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Plugin_Message
template<>
void client::handle_packet<proto::generated::clientbound::play::custom_payload_packet>(proto::packet_reader &s) {
    std::string channel = s.read_identifier();
    if (channel == "minecraft:brand") {
        MCCPP_I("Server Brand: {}", s.read_char_array(s.remaining()));
    } else {
        MCCPP_I("Received plugin message of length {} on an unknown channel \"{}\"", s.remaining(), channel);
        s.discard(s.remaining());
    }
}

}
