#include <iostream>

#include "application.hh"

int main(int argc, char **argv)
{
    std::clog.sync_with_stdio(false);
    return mccpp::application::main(argc, argv);
}
