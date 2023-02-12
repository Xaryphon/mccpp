#include <iostream>

#include "application.hh"

#if 0

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

    auto client = std::make_unique<client::client>();
    client->connect(io, "127.0.0.1", 25564);
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
