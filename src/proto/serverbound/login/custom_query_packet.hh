#pragma once

#include <string>
#include <optional>

#include "generated/proto/misc.hh"
#include "generated/proto/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Login_Plugin_Response
template<>
struct packet<generated::serverbound::login::custom_query_packet> {
    using packet_type = generated::serverbound::login::custom_query_packet;

    int32_t message_id;

    void write(packet_writer &s) const {
        s.write_varint(message_id);
        s.write_bool(false);
    }
};

}
