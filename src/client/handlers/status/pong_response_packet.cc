#include "../../handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

template<>
void packet_handler<proto::generated::clientbound::status::pong_response_packet>::process() {
    MCCPP_I("pong response: 0x{:016x}", packet().payload);
}

}
