#pragma once

#include "../../generated/misc.hh"
#include "../../generated/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

template<>
struct packet<generated::serverbound::status::status_request_packet> {
    using packet_type = generated::serverbound::status::status_request_packet;

    void write(packet_writer &) const {
    }
};

}
