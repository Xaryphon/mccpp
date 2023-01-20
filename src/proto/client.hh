#pragma once

#include "packet.hh"
#include "tcp_client.hh"

namespace mccpp::proto {

class client : private tcp_client {
public:
    using create_handler_fn = std::function<std::unique_ptr<packet_handler>(int32_t)>;

    client(asio::ip::tcp::socket &&socket, create_handler_fn &&handler)
    : tcp_client(std::move(socket))
    , m_create_handler(std::move(handler))
    , m_receive_task(receiver_task())
    {
        m_receive_task.handle().resume();
    }

    template<typename PacketInfo>
    void queue_send(const packet<PacketInfo> &p) {
        packet_writer w {};
        w.write_varint(PacketInfo::id);
        p.write(w);
        queue_send(w);
    }

private:
    task<int32_t> async_read_varint();
    void write_varint(int32_t);

    void queue_send(std::span<const std::byte> body);

    virtual void on_readable() override;

    task<> receiver_task();

    create_handler_fn m_create_handler;
    task<> m_receive_task;
};

}
