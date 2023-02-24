#pragma once

#include "generated/proto/misc.hh"
#include "generated/proto/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Plugin_Message_2
template<>
struct packet<generated::serverbound::play::custom_payload_packet> {
    using packet_type = generated::serverbound::play::custom_payload_packet;

    std::string channel;
    std::vector<std::byte> data;

    void write(packet_writer &s) const {
        s.write_identifier(channel);
        s.write_bytes(data);
    }
};

}
