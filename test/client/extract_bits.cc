#include <catch2/catch_test_macros.hpp>

#include "utility/extract_bits.hh"

TEST_CASE("extract_bits", "[client]") {
    using namespace mccpp;
    std::vector<uint64_t> data { 0x0123456789abcdefULL, 0x1032547698badcfeULL, 0 };
    REQUIRE(extract_bits(data, 0, 8) == 0x01);
    REQUIRE(extract_bits(data, 4, 8) == 0x12);
    REQUIRE(extract_bits(data, 8, 8) == 0x23);
    REQUIRE(extract_bits(data, 12, 8) == 0x34);
    REQUIRE(extract_bits(data, 16, 8) == 0x45);
    REQUIRE(extract_bits(data, 20, 8) == 0x56);
    REQUIRE(extract_bits(data, 24, 8) == 0x67);
    REQUIRE(extract_bits(data, 28, 8) == 0x78);
    REQUIRE(extract_bits(data, 32, 8) == 0x89);
    REQUIRE(extract_bits(data, 36, 8) == 0x9a);
    REQUIRE(extract_bits(data, 40, 8) == 0xab);
    REQUIRE(extract_bits(data, 44, 8) == 0xbc);
    REQUIRE(extract_bits(data, 48, 8) == 0xcd);
    REQUIRE(extract_bits(data, 52, 8) == 0xde);
    REQUIRE(extract_bits(data, 56, 8) == 0xef);
    REQUIRE(extract_bits(data, 60, 8) == 0xf1);
    REQUIRE(extract_bits(data, 64, 8) == 0x10);
    REQUIRE(extract_bits(data, 68, 8) == 0x03);
    REQUIRE(extract_bits(data, 72, 8) == 0x32);
    REQUIRE(extract_bits(data, 76, 8) == 0x25);
    REQUIRE(extract_bits(data, 80, 8) == 0x54);
    REQUIRE(extract_bits(data, 84, 8) == 0x47);
    REQUIRE(extract_bits(data, 88, 8) == 0x76);
    REQUIRE(extract_bits(data, 92, 8) == 0x69);
    REQUIRE(extract_bits(data, 96, 8) == 0x98);
    REQUIRE(extract_bits(data, 100, 8) == 0x8b);
    REQUIRE(extract_bits(data, 104, 8) == 0xba);
    REQUIRE(extract_bits(data, 108, 8) == 0xad);
    REQUIRE(extract_bits(data, 112, 8) == 0xdc);
    REQUIRE(extract_bits(data, 116, 8) == 0xcf);
    REQUIRE(extract_bits(data, 120, 8) == 0xfe);
}
