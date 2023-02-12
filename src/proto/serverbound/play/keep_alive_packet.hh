#pragma once

#include "../../generated/misc.hh"
#include "../../generated/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Keep_Alive_2
template<>
struct packet<generated::serverbound::play::keep_alive_packet> {
    using packet_type = generated::serverbound::play::keep_alive_packet;

    int64_t keep_alive_id;

    void write(packet_writer &s) const {
        s.write_i64(keep_alive_id);
    }
};

}
