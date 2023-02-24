#pragma once

#include <string>
#include <optional>

#include "generated/proto/misc.hh"
#include "generated/proto/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Login_Start
template<>
struct packet<generated::serverbound::login::hello_packet> {
    using packet_type = generated::serverbound::login::hello_packet;

    std::string name;
    std::optional<class uuid> uuid;

    void write(packet_writer &s) const {
        s.write_string<16>(name);
        s.write_bool(uuid.has_value());
        if (uuid.has_value())
            s.write_uuid(*uuid);
    }
};

}
