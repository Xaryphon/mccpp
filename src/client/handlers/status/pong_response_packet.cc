#include "generated/client/handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Ping_Response
template<>
void client::handle_packet<proto::generated::clientbound::status::pong_response_packet>(proto::packet_reader &s) {
    MCCPP_I("pong response: 0x{:016x}", s.read_i64());
}

}
