#pragma once

#include <memory>

#include "../proto/client.hh"
#include "../proto/generated/misc.hh"
#include "../proto/packet.hh"

namespace mccpp::client {

class client : public proto::client {
    using connection_state = proto::generated::connection_state;

public:
    client(asio::ip::tcp::socket &&socket)
    : proto::client(std::move(socket))
    , m_state(connection_state::HANDSHAKING)
    {}

    void set_state(proto::generated::connection_state new_state) {
        m_state = new_state;
    }

private:
    // NOTE: implemented in handlers.cc
    void on_packet_received(int32_t, proto::packet_reader &) override final;

    // NOTE: implemented in handlers.cc and handlers/**/*.cc
    template<class PacketInfo>
    void handle_packet(proto::packet_reader &);

    connection_state m_state;

    template<typename T>
    friend class packet_handler;
};

}
