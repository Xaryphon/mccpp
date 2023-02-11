#pragma once

#include <memory>

#include "../proto/client.hh"
#include "../proto/generated/misc.hh"
#include "../proto/packet.hh"

namespace mccpp::client {

class client final : public proto::client {
    using connection_state = proto::generated::connection_state;

public:
    void connect(asio::io_context &, std::string_view address, uint16_t port);

    void set_state(proto::generated::connection_state new_state) {
        m_state = new_state;
    }

private:
    void on_error() override final;
    void on_connect() override final;

    // NOTE: implemented in handlers.cc
    void on_packet_received(int32_t, proto::packet_reader &) override final;

    // NOTE: implemented in handlers.cc and handlers/**/*.cc
    template<class PacketInfo>
    void handle_packet(proto::packet_reader &);

    connection_state m_state = connection_state::HANDSHAKING;

    std::string m_server_name;
    uint16_t m_server_port;

    template<typename T>
    friend class packet_handler;
};

}
