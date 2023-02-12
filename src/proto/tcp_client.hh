#pragma once

#include <span>

#include <asio.hpp>

#include "../utility/coro.hh"
#include "../utility/ring_buffer.hh"

namespace mccpp::proto {

class tcp_client {
public:
    using tcp = asio::ip::tcp;

    class reader {
    public:
        bool await_ready() { return m_client.m_read_buffer.readable() >= 1; }
        void await_suspend(std::coroutine_handle<> handle) {
            return m_client.m_read_buffer.resume_on_readable(handle);
        }
        std::byte await_resume() {
            return m_client.m_read_buffer.pop_front();
        }

    private:
        reader(tcp_client &client)
        : m_client(client)
        {}

        tcp_client &m_client;

        friend class tcp_client;
    };

    void connect(asio::io_context &, tcp::endpoint);

    std::byte read_byte() { return m_read_buffer.pop_front(); }
    reader async_read_byte() { return { *this }; }
    task<> async_recv_until(size_t n);

    void write_bytes(std::span<const std::byte>);
    void write_byte(std::byte b) { write_bytes({ &b, 1 }); }
    void write_flush();

protected:
    virtual void on_tcp_error(asio::error_code) = 0;
    virtual void on_tcp_connect() = 0;
    virtual void on_readable() = 0;

private:
    class buffer {
    public:
        size_t capacity() { return m_buffer.capacity(); }
        size_t readable() { return m_buffer.readable(); }
        void resume_on_readable(std::coroutine_handle<> h);
        std::byte pop_front();

        buffer(tcp_client &c)
        : m_client(c)
        {}

    private:
        tcp_client &m_client;

        ring_buffer<2097151> m_buffer;
        std::coroutine_handle<> m_resume = nullptr;
        friend class tcp_client;
    } m_read_buffer = { *this };

    std::optional<tcp::socket> m_socket;
    std::vector<std::byte> m_write_buffer;
};

}
