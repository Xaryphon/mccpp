#include "../../handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

template<>
void client::handle_packet<status::pong_response_packet>(proto::packet_reader &s) {
    MCCPP_I("pong response: 0x{:016x}", s.read_i64());
}

}
