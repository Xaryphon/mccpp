#pragma once

#include "client.hh"
#include "../proto/generated/clientbound/types.hh"

namespace mccpp::client {

using namespace proto::generated::clientbound;

template<> void client::handle_packet<status::pong_response_packet>(proto::packet_reader &);
template<> void client::handle_packet<status::status_response_packet>(proto::packet_reader &);

}
