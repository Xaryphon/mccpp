#!/bin/sh

set -e

cmake --build build
ctest --test-dir build/test "$@"
