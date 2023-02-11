#include "../../handlers.hh"

#include "../../../logger.hh"
#include "../../../proto/serverbound/packets.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Login_Plugin_Request
template<>
void client::handle_packet<proto::generated::clientbound::login::custom_query_packet>(proto::packet_reader &s) {
    using namespace proto::generated;

    int32_t message_id = s.read_varint();
    s.discard(s.remaining());
    queue_send<serverbound::login::custom_query_packet>({
        .message_id = message_id,
    });
}

}
