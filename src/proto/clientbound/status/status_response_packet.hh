#pragma once

#include <string>

#include "../../generated/misc.hh"
#include "../../generated/clientbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

template<>
struct packet<generated::clientbound::status::status_response_packet> {
    using packet_type = generated::clientbound::status::status_response_packet;

    std::string json_response;

    void read(packet_reader &s) {
        json_response = s.read_string<32767>();
    }
};

}
