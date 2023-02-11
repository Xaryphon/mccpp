#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <span>
#include <string_view>
#include <string>
#include <vector>

#include "../utility/coro.hh"
#include "../uuid.hh"

namespace mccpp::proto {

class packet_writer {
public:
    packet_writer() = default;

    void write_varint(int32_t);
    void write_bytes(std::span<const std::byte>);
    void write_bool(bool);
    void write_u16(uint16_t);
    void write_u64(uint64_t);
    void write_i64(int64_t);

    void write_uuid(uuid uuid) {
        write_u64(uuid.high());
        write_u64(uuid.low());
    }

    template<size_t MaxCodePoints>
    void write_string(std::string_view str) {
        static_assert(MaxCodePoints <= 32767);
        write_string(str, MaxCodePoints);
    }

    operator std::span<const std::byte>() const {
        return m_buffer;
    }

private:
    void write_bytes(std::span<const std::byte>, bool reverse);
    void write_string(std::string_view, size_t max_code_points);

    template<typename T>
    void write_as_bytes(std::span<T> span, bool reverse) {
        write_bytes(std::as_bytes(span), reverse);
    }

    std::vector<std::byte> m_buffer;
};

class packet_reader {
public:
    using read_byte_fn = std::function<std::byte()>;

    packet_reader(read_byte_fn &&read_byte, size_t packet_length)
    : m_remaining(packet_length)
    , m_read_byte(std::move(read_byte))
    {}

    size_t remaining() { return m_remaining; }
    void discard(size_t);

    std::byte read_byte();
    int32_t read_varint();
    bool read_bool();
    int64_t read_u64();
    int64_t read_i64();

    uuid read_uuid() {
        uint64_t high = read_u64();
        uint64_t low = read_u64();
        return { high, low };
    }

    template<size_t MaxCodePoints>
    std::string read_string() {
        static_assert(MaxCodePoints <= 32767);
        return read_string(MaxCodePoints);
    }

private:
    template<typename T>
    T read_int_n();

    std::string read_string(size_t max_code_points);

    size_t m_remaining;
    read_byte_fn m_read_byte;
};

template<typename TPacket>
struct packet;

}
