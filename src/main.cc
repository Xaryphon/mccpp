#include <iostream>

#include "application.hh"

int main(int argc, char **argv) {
    // Clear the terminal to work around VSCode/CodeLLDB bullshit
    // FIXME: Figure out an alternative solution for this
    std::clog << "\033[3J" << std::flush;
    std::clog.sync_with_stdio(false);
    return mccpp::main(argc, argv);
}
