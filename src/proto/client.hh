#pragma once

#include "packet.hh"
#include "tcp_client.hh"

namespace mccpp::proto {

class client : private tcp_client {
public:
    client()
    : m_receive_task(receiver_task())
    {}

    void connect(asio::io_context &, std::string_view address, uint16_t port);

    template<typename PacketInfo>
    void queue_send(const packet<PacketInfo> &p) {
        packet_writer w {};
        w.write_varint(PacketInfo::id);
        p.write(w);
        queue_send(w);
    }

protected:
    virtual void on_error() = 0;
    virtual void on_connect() = 0;
    virtual void on_packet_received(int32_t, packet_reader &) = 0;

private:
    void on_tcp_error(asio::error_code) override final;
    void on_tcp_connect() override final;

    task<int32_t> async_read_varint();
    void write_varint(int32_t);

    void queue_send(std::span<const std::byte> body);

    virtual void on_readable() override;

    task<> receiver_task();

    task<> m_receive_task;
};

}
