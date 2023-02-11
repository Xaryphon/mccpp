#include "client.hh"

#include "../proto/serverbound/handshaking/client_intention_packet.hh"

namespace mccpp::client {

void client::connect(asio::io_context &io, std::string_view address, uint16_t port) {
    m_server_name = address;
    m_server_port = port;
    proto::client::connect(io, address, port);
}

void client::on_error() {
    abort();
}

void client::on_connect() {
    using namespace proto::generated::serverbound;
    queue_send<handshaking::client_intention_packet>({
        .protocol_version = 791,
        .server_address = m_server_name,
        .server_port = m_server_port,
        .next_state = proto::packet<handshaking::client_intention_packet>::STATUS,
    });
    m_state = connection_state::STATUS;
}

}
