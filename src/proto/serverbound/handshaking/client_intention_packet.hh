#pragma once


#include <string>

#include "../../generated/misc.hh"
#include "../../generated/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Handshake
template<>
struct packet<generated::serverbound::handshaking::client_intention_packet> {
    using packet_type = generated::serverbound::handshaking::client_intention_packet;

    // these map to generated::connection_state
    enum next_state {
        STATUS = 1,
        LOGIN = 2,
    };

    int32_t protocol_version;
    std::string server_address;
    uint16_t server_port;
    next_state next_state;

    void write(packet_writer &s) const {
        s.write_varint(protocol_version);
        s.write_string<255>(server_address);
        s.write_u16(server_port);
        s.write_varint(next_state);
    }
};

}
