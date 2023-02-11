#include "handlers.hh"

#include "../logger.hh"
#include "../proto/exceptions.hh"
#include "../proto/generated/clientbound/iterators.hh"
#include "../proto/generated/clientbound/traits.hh"
#include "../proto/generated/format.hh"
#include "client.hh"
#include "handlers.hh"

namespace mccpp::client {

template<class PacketInfo>
void client::handle_packet(proto::packet_reader &s) {
    using namespace proto::generated;
    MCCPP_T("ignoring an unimplemented packet {}", packet_traits<PacketInfo>::name);
    s.discard(s.remaining());
}

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
