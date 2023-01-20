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
    : proto::client(std::move(socket), [this](int32_t id) { return create_handler(id); })
    , m_state(connection_state::HANDSHAKING)
    {}

    // NOTE: implemented in handler.cc
    std::unique_ptr<proto::packet_handler> create_handler(int32_t packet_id);

    void set_state(proto::generated::connection_state new_state) {
        m_state = new_state;
    }

private:
    connection_state m_state;

    template<typename T>
    friend class packet_handler;
};

}
