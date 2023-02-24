#include "generated/client/handlers.hh"

#include "logger.hh"
#include "proto/exceptions.hh"
#include "generated/proto/clientbound/iterators.hh"
#include "generated/proto/clientbound/traits.hh"
#include "generated/proto/format.hh"
#include "client.hh"

namespace mccpp::client {

template<class PacketInfo>
void client::handle_packet(proto::packet_reader &s) {
    using namespace proto::generated;
    MCCPP_T("ignoring an unimplemented packet {}", packet_traits<PacketInfo>::name);
    s.discard(s.remaining());
}

#define SILENCE_PACKET(name) \
        template<> \
        void client::handle_packet<proto::generated::clientbound::play::name>(proto::packet_reader &s) { \
            s.discard(s.remaining()); \
        }

SILENCE_PACKET(add_entity_packet)
SILENCE_PACKET(entity_event_packet)
SILENCE_PACKET(pos_rot)
SILENCE_PACKET(pos)
SILENCE_PACKET(remove_entities_packet)
SILENCE_PACKET(rot)
SILENCE_PACKET(rotate_head_packet)
SILENCE_PACKET(set_entity_data_packet)
SILENCE_PACKET(set_entity_motion_packet)
SILENCE_PACKET(set_time_packet)
SILENCE_PACKET(teleport_entity_packet)
SILENCE_PACKET(update_attributes_packet)

void client::on_packet_received(int32_t packet_id, proto::packet_reader &reader) {
    using namespace proto::generated;
    switch (m_state) {
#define _MCCPP_ITER_STATE_START(state)  \
    case connection_state::state: \
        switch (packet_id) {
#define _MCCPP_ITER_PACKET(type) \
            case type::id: return handle_packet<type>(reader);
#define _MCCPP_ITER_STATE_END(state) \
        } \
        break;

    MCCPP_ITERATE_CLIENTBOUND

#undef _MCCPP_ITER_STATE_END
#undef _MCCPP_ITER_PACKET
#undef _MCCPP_ITER_STATE_START
    };

    // Either the server sent us an invalid packet or we're in an incorrect state
    MCCPP_E("Received an invalid packet id 0x{:02x} in state {}", packet_id, m_state);
    throw proto::protocol_error("invalid packet id");
}

}
