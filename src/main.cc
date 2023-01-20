#include <iostream>

#include "application.hh"

#if 1

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

    using asio::ip::tcp;
    tcp::socket socket { io };
    socket.connect(tcp::endpoint { asio::ip::make_address("127.0.0.1"), 25564 });

    client::client client { std::move(socket) };
    using namespace mccpp::proto;
    using namespace mccpp::proto::generated;

    client.queue_send(packet<serverbound::handshaking::client_intention_packet> {
        .protocol_version = 761,
        .server_address = "127.0.0.1",
        .server_port = 25564,
        .next_state = packet<serverbound::handshaking::client_intention_packet>::STATUS,
    });
    client.set_state(connection_state::STATUS);

    client.queue_send(packet<serverbound::status::status_request_packet> {});
    client.queue_send(packet<serverbound::status::ping_request_packet> {
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
    return mccpp::application::main(argc, argv);
}

#endif
