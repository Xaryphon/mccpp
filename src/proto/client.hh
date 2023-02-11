#pragma once

#include "packet.hh"
#include "tcp_client.hh"

namespace mccpp::proto {

class client : private tcp_client {
public:
    client(asio::ip::tcp::socket &&socket)
    : tcp_client(std::move(socket))
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

protected:
    virtual void on_packet_received(int32_t, packet_reader &) = 0;

private:
    task<int32_t> async_read_varint();
    void write_varint(int32_t);

    void queue_send(std::span<const std::byte> body);

    virtual void on_readable() override;

    task<> receiver_task();

    task<> m_receive_task;
};

}
