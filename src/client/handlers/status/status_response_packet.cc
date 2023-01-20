#include "../../handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

template<>
void packet_handler<proto::generated::clientbound::status::status_response_packet>::process() {
    MCCPP_I("server status json: {}", packet().json_response);
}

}
