#pragma once

#include <memory>

#include "../game.hh"
#include "../proto/client.hh"
#include "../proto/generated/misc.hh"
#include "../proto/packet.hh"

namespace mccpp::client {

class client final : public proto::client {
    using connection_state = proto::generated::connection_state;

public:
    client(application &app)
    : m_game(app.game())
    {}

    void connect(asio::io_context &, std::string_view address, uint16_t port);

private:
    void on_error() override final;
    void on_connect() override final;

    // NOTE: implemented in handlers.cc
    void on_packet_received(int32_t, proto::packet_reader &) override final;

    // NOTE: implemented in handlers.cc and handlers/**/*.cc
    template<class PacketInfo>
    void handle_packet(proto::packet_reader &);

    game &m_game;

    connection_state m_state = connection_state::HANDSHAKING;

    std::string m_server_name;
    uint16_t m_server_port;

    template<typename T>
    friend class packet_handler;
};

}
