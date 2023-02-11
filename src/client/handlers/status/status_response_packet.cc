#include "../../handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

template<>
void client::handle_packet<status::status_response_packet>(proto::packet_reader &s) {
    MCCPP_I("server status json: {}", s.read_string<32767>());
}

}
