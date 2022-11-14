#!/bin/sh

set -e

cmake -S . -B build -GNinja -Werror=dev \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_FLAGS="-Wall -Wpedantic -Wextra -Werror" \
    -DCMAKE_CXX_DEBUG_FLAGS="-fsanitize=address,undefined" \
    "$@"
