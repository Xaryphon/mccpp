#!/bin/sh

set -e


CXXFLAGS="$CXXFLAGS -Wall -Wpedantic -Wextra -Werror"

# Building glm on c++20 requires this
CXXFLAGS="$CXXFLAGS -Wno-volatile"

cmake -S . -B build -GNinja -Werror=dev \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
    -DCMAKE_CXX_FLAGS_DEBUG="-g -fsanitize=address,undefined" \
    "$@"
