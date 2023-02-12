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

struct position {
public:
    position() : m_data(0) {}
    position(uint64_t raw) : m_data(raw) {}
    position(int64_t x, int64_t z) : position(x, 0, z) {}
    position(int64_t x, int64_t y, int64_t z)
    : m_data(((x & 0x3ffffff) << 38) | ((z & 0x3ffffff) << 12) | (y & 0xfff))
    {}

    uint64_t raw() { return m_data; }
    int64_t x() { return m_data >> 38 & 0x3ffffff; }
    int64_t z() { return m_data >> 12 & 0x3ffffff; }
    int64_t y() { return m_data       & 0xfff;     }

private:
    uint64_t m_data;
};

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
    uint8_t read_u8();
    uint16_t read_u16();
    uint32_t read_u32();
    uint64_t read_u64();
    int8_t read_i8();
    int16_t read_i16();
    int32_t read_i32();
    int64_t read_i64();
    float read_float();
    double read_double();

    std::string read_identifier() { return read_string<32767>(); }
    position read_position() { return { read_u64() }; }

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

    std::vector<std::byte> read_byte_array(size_t n);
    std::string read_char_array(size_t n);

private:
    template<typename T>
    T read_int_n();

    template<typename T>
    T read_float_n();

    std::string read_string(size_t max_code_points);

    size_t m_remaining;
    read_byte_fn m_read_byte;
};

template<typename TPacket>
struct packet;

}
