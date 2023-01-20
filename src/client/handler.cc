#include "handler.hh"

#include "../logger.hh"
#include "../proto/exceptions.hh"
#include "../proto/generated/clientbound/iterators.hh"
#include "../proto/generated/clientbound/traits.hh"
#include "../proto/generated/format.hh"
#include "client.hh"
#include "handlers.hh"

// FIXME: This feels really hacky :/
template<typename TPacket>
struct mccpp::proto::packet {
    void read(proto::packet_reader &s) {
        while (s.remaining() > 0) {
            s.read_byte();
        }
    }
};

namespace mccpp::client {

template<typename T>
void packet_handler<T>::process() {
    using namespace proto::generated;
    MCCPP_T("ignoring an unimplemented packet {}", packet_traits<T>::name);
}

std::unique_ptr<proto::packet_handler> client::create_handler(int32_t packet_id) {
    using namespace proto::generated;
    switch (m_state) {
#define _MCCPP_ITER_STATE_START(state)  \
    case connection_state::state: \
        switch (packet_id) {
#define _MCCPP_ITER_PACKET(type) \
            case type::id: return std::make_unique<packet_handler<type>>(*this);
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
