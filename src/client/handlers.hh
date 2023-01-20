#pragma once

#include "handler.hh"
#include "../proto/clientbound/packets.hh"

namespace mccpp::client {

using namespace proto::generated::clientbound;

template<> void packet_handler<status::pong_response_packet>::process();
template<> void packet_handler<status::status_response_packet>::process();

}
