// Automatically generated by generator/packet_types.py
#pragma once

namespace mccpp::proto::generated {

enum class packet_direction {
 SERVERBOUND,
 CLIENTBOUND,
};

enum class connection_state {
 HANDSHAKING = -1,
 PLAY = 0,
 STATUS = 1,
 LOGIN = 2,
};

template<typename T>
struct packet_traits;

}