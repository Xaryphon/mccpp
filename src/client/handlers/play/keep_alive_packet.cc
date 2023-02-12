#include "../../handlers.hh"

#include "../../../logger.hh"
#include "../../../proto/serverbound/packets.hh"
#include "../../../utility/format.hh"
#include "../../../nbt.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Keep_Alive
template<>
void client::handle_packet<proto::generated::clientbound::play::keep_alive_packet>(proto::packet_reader &s) {
    using namespace proto::generated;

    int64_t keep_alive_id = s.read_i64();
    queue_send<serverbound::play::keep_alive_packet>({
        .keep_alive_id = keep_alive_id,
    });
}

}
