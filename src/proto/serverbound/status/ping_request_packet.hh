#pragma once

#include "../../generated/misc.hh"
#include "../../generated/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Ping_Request
template<>
struct packet<generated::serverbound::status::ping_request_packet> {
    using packet_type = generated::serverbound::status::ping_request_packet;

    int64_t payload;

    void write(packet_writer &s) const {
        s.write_i64(payload);
    }
};

}
