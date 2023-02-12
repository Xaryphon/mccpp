#include "tcp_client.hh"

#include "../logger.hh"

namespace mccpp::proto {

void tcp_client::connect(asio::io_context &io, tcp::endpoint endpoint) {
    m_socket.emplace(io);
    // TODO: Make async
    m_socket->connect(endpoint);
    on_tcp_connect();
}

task<> tcp_client::async_recv_until(size_t n) {
    assert(n <= m_read_buffer.capacity());

    struct {
        constexpr bool await_ready() noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle) {
            client.m_read_buffer.resume_on_readable(handle);
        }
        constexpr void await_resume() noexcept {}

        tcp_client &client;
    } awaiter { *this };

    while (m_read_buffer.readable() < n) {
        co_await awaiter;
    }
}

void tcp_client::write_bytes(std::span<const std::byte> bytes) {
    m_write_buffer.insert(m_write_buffer.end(), bytes.begin(), bytes.end());
}

void tcp_client::write_flush() {
    // FIXME: Flush write buffer asynchronously
    for (size_t offset = 0; offset < m_write_buffer.size();) {
        offset += m_socket->write_some(asio::buffer(m_write_buffer.data() + offset, m_write_buffer.size() - offset));
    }
    m_write_buffer.clear();
}

static asio::mutable_buffer span_to_asio(std::span<std::byte> buf) {
    return { buf.data(), buf.size() };
}

void tcp_client::buffer::resume_on_readable(std::coroutine_handle<> h) {
    assert(!m_resume);
    m_resume = h;
    asio::mutable_buffer front = span_to_asio(m_buffer.write_front());
    asio::mutable_buffer back = span_to_asio(m_buffer.write_back());
    m_client.m_socket->async_read_some(std::array {front, back},
        [this](const asio::error_code& error, std::size_t bytes_read)
    {
        if (error == asio::error::eof) {
            MCCPP_D("EOF");
            // FIXME: throw eof
        } else if (error) {
            MCCPP_E("read failed: {}", error.message());
            // FIXME: throw error
        } else {
            m_buffer.mark_write(bytes_read);
            if (m_resume) {
                std::coroutine_handle<> handle = m_resume;
                m_resume = nullptr;
                handle.resume();
            }
            m_client.on_readable();
        }
    });
}

std::byte tcp_client::buffer::pop_front() {
    std::byte b = m_buffer.front();
    m_buffer.erase(1);
    return b;
}

}
