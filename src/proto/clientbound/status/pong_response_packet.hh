#pragma once

#include <string>

#include "../../generated/misc.hh"
#include "../../generated/clientbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

template<>
struct packet<generated::clientbound::status::pong_response_packet> {
    using packet_type = generated::clientbound::status::pong_response_packet;

    int64_t payload;

    void read(packet_reader &s) {
        payload = s.read_i64();
    }
};

}
