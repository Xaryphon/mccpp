#include "../../handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Status_Response
template<>
void client::handle_packet<proto::generated::clientbound::status::status_response_packet>(proto::packet_reader &s) {
    MCCPP_I("server status json: {}", s.read_string<32767>());
}

}
