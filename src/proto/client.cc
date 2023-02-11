#include "client.hh"

#include "../logger.hh"
#include "../utility/format.hh"
#include "varint.hh"

namespace mccpp::proto {

void client::connect(asio::io_context &io, std::string_view address, uint16_t port) {
    tcp_client::connect(io, asio::ip::tcp::endpoint { asio::ip::make_address(address), port });
}

void client::on_tcp_error(asio::error_code error) {
    MCCPP_E("TCP connect failed: {}", error.message());
}

void client::on_tcp_connect() {
    MCCPP_I("TCP connect success");
    m_receive_task.handle().resume();
    on_connect();
}

void client::write_varint(int32_t value) {
    varint::write(value, [this](std::byte byte) { write_byte(byte); });
}

void client::queue_send(std::span<const std::byte> body) {
    // Packets cannot be larger than 2^21 − 1 or 2097151 bytes (the maximum that can be sent in a 3-byte VarInt). For compressed packets, this applies to both the compressed length and uncompressed lengths.
    if (body.size() > 2097151) {
        throw encode_error("Packet length exceeded");
    }

    MCCPP_T("Sending packet of length {}:\n{:c}", body.size(), body);

    int32_t packet_length = body.size();
    write_varint(packet_length);
    write_bytes(body);
    write_flush();

    //if (m_compress_threshold < 0) {
    //} else if (m_compress_threshold < body.size()) {
    //    write_varint(packet_length);
    //    write_varint(0);
    //    write_bytes(body);
    //} else {
    //    // TODO: Send compressed packet
    //    abort();
    //    /*  Pseudo code
    //     *  auto buffer = compress(body);
    //     *  write_varint(buffer.size());
    //     *  write_varint(packet_length);
    //     *  write_bytes(buffer);
    //     */
    //}
}

task<int32_t> client::async_read_varint() {
    return varint::async_read([this] {
        return async_read_byte();
    });
}

void client::on_readable() {
    if (m_receive_task.handle().done()) {
        m_receive_task.await_resume();
    }
}

task<> client::receiver_task() {
    for (;;) {
        int32_t packet_length = co_await async_read_varint();
        if (packet_length <= 0) {
            throw decode_error("invalid packet length (too low)");
        }
        if (packet_length > 2097151) {
            throw decode_error("invalid packet length (too high)");
        }

        MCCPP_T("Receiving a packet of length {}", packet_length);
        co_await async_recv_until(packet_length);
        //MCCPP_T("Received packet of length {}:\n{:c}", packet_length, peek_all().subspan(0, packet_length));

        packet_reader reader { [this] {
            return read_byte();
        }, size_t(packet_length) };

        int32_t packet_id = reader.read_varint();
        on_packet_received(packet_id, reader);
        if (reader.remaining() != 0) {
            throw decode_error("trailing data in packet");
        }
    }
}

}
