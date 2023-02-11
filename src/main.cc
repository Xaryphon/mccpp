#include <iostream>

#include "application.hh"

#if 0

#include <utility>

struct foo {
    foo(int i = 0) : value(i) { std::cout << value << __func__ << std::endl; }
    ~foo() { std::cout << value << __func__ << std::endl; }

    foo(foo &&other) : value(std::exchange(other.value, 0)) {}
    foo &operator=(foo &&other) {
        value = std::exchange(other.value, 0);
        return *this;
    }

    int value;
};

struct bar {
    bar()
    {
        foo f { 1 };
        (void)f;
        m_foo = { 2 };
        throw std::exception();
    }

    foo m_foo;
};

int main() {
    try {
        bar b {};
        (void)b;
    } catch (const std::exception &) {

    }
}

#elif 0

#include "logger.hh"
#include <cassert>
#include "utility/format.hh"

#include <asio.hpp>
#include <optional>
#include <coroutine>
#include <concepts>
#include <deque>

#include "client/client.hh"
#include "utility/coro.hh"
#include "proto/client.hh"
#include "proto/serverbound/packets.hh"

int main(int argc, char **argv) {
    // Clear the terminal to work around VSCode/CodeLLDB bullshit
    std::cout << "\033[3J" << std::flush;

    using namespace mccpp;
    logger::set_thread_name("main");

    asio::io_context io;

    client::client client {};
    client.connect(io, "127.0.0.1", 25564);

    using namespace mccpp::proto::generated;

    client.queue_send<serverbound::status::status_request_packet>({});
    client.queue_send<serverbound::status::ping_request_packet>({
        .payload = 0x012345678,
    });

    io.run();

    (void)argc;
    (void)argv;
    return 0;
}

#else

int main(int argc, char **argv)
{
    std::clog.sync_with_stdio(false);
    return mccpp::main(argc, argv);
}

#endif
