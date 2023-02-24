#pragma once

#include "generated/proto/misc.hh"
#include "generated/proto/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Status_Request
template<>
struct packet<generated::serverbound::status::status_request_packet> {
    using packet_type = generated::serverbound::status::status_request_packet;

    void write(packet_writer &) const {
    }
};

}
