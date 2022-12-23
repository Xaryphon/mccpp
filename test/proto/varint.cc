#include <catch2/catch_test_macros.hpp>

#include <cassert>
#include <cstddef>
#include <span>
#include <queue>
#include <array>

#include "proto/varint.hh"

template<typename ...Args, size_t N = sizeof...(Args)>
std::array<std::byte, N> byte_array(Args ...args) {
    return { std::byte(args)... };
}

template<typename TS>
void test_varint(TS value, std::span<const std::byte> expected,
                  std::deque<std::byte> &buffer)
{
    using namespace mccpp::proto;
    DYNAMIC_SECTION("value: " << value) {
        varint_impl::write_impl<TS>(value, [&buffer](std::byte b) {
            buffer.emplace_back(b);
        });

        REQUIRE(buffer.size() == expected.size());
        for (size_t i = 0; i < buffer.size(); i++) {
            REQUIRE(buffer[i] == expected[i]);
        }

        TS decoded = varint_impl::read_impl<TS>([&buffer]() {
            auto v = buffer.front();
            buffer.pop_front();
            return v;
        });

        REQUIRE(buffer.empty());
        REQUIRE(decoded == value);
    }
}

TEST_CASE("varint", "[proto][varint]") {
    std::deque<std::byte> buffer;

    test_varint<int32_t>(0, byte_array(0x00), buffer);
    test_varint<int32_t>(1, byte_array(0x01), buffer);
    test_varint<int32_t>(2, byte_array(0x02), buffer);
    test_varint<int32_t>(127, byte_array(0x7f), buffer);
    test_varint<int32_t>(128, byte_array(0x80, 0x01), buffer);
    test_varint<int32_t>(255, byte_array(0xff, 0x01), buffer);
    test_varint<int32_t>(25565, byte_array(0xdd, 0xc7, 0x01), buffer);
    test_varint<int32_t>(2097151, byte_array(0xff, 0xff, 0x7f), buffer);
    test_varint<int32_t>(2147483647, byte_array(0xff, 0xff, 0xff, 0xff, 0x07), buffer);
    test_varint<int32_t>(-1, byte_array(0xff, 0xff, 0xff, 0xff, 0x0f), buffer);
    test_varint<int32_t>(-2147483648, byte_array(0x80, 0x80, 0x80, 0x80, 0x08), buffer);
}

TEST_CASE("varlong", "[proto][varint]") {
    std::deque<std::byte> buffer;

    test_varint<int64_t>(0, byte_array(0x00), buffer);
    test_varint<int64_t>(1, byte_array(0x01), buffer);
    test_varint<int64_t>(2, byte_array(0x02), buffer);
    test_varint<int64_t>(127, byte_array(0x7f), buffer);
    test_varint<int64_t>(128, byte_array(0x80, 0x01), buffer);
    test_varint<int64_t>(255, byte_array(0xff, 0x01), buffer);
    test_varint<int64_t>(2147483647, byte_array(0xff, 0xff, 0xff, 0xff, 0x07), buffer);
    test_varint<int64_t>(9223372036854775807, byte_array(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f), buffer);
    test_varint<int64_t>(-1, byte_array(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01), buffer);
    test_varint<int64_t>(-2147483648, byte_array(0x80, 0x80, 0x80, 0x80, 0xf8, 0xff, 0xff, 0xff, 0xff, 0x01), buffer);
    test_varint<int64_t>(-9223372036854775807 - 1, byte_array(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01), buffer);
}
