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
    position() : m_x(0), m_z(0), m_y(0) {}

    position(uint64_t raw)
    : m_x(sign_extend<26>(raw >> 38 & 0x3ffffff))
    , m_z(sign_extend<26>(raw >> 12 & 0x3ffffff))
    , m_y(sign_extend<12>(raw & 0xfff))
    {}

    position(int32_t x, int32_t z) : position(x, 0, z) {}
    position(int32_t x, int32_t y, int32_t z) : m_x(x), m_z(z), m_y(y) {}

    uint64_t raw() {
        return ((int64_t(m_x) & 0x3ffffff) << 38)
             | ((int64_t(m_z) & 0x3ffffff) << 12)
             | ((int64_t(m_y) & 0xfff));
    }

    int32_t x() { return m_x; }
    int32_t z() { return m_z; }
    int32_t y() { return m_y; }

private:
    // Yoink'd from https://stackoverflow.com/questions/58904102/how-to-safely-extract-a-signed-field-from-a-uint32-t-into-a-signed-number-int-o
    template <uint32_t N>
    int32_t sign_extend(uint32_t value) {
        static_assert(N > 0 && N <= 32);
        constexpr uint32_t unusedBits = (uint32_t(32) - N);
        if constexpr (int32_t(0xFFFFFFFFu) >> 1 == int32_t(0xFFFFFFFFu)) {
            return int32_t(value << unusedBits) >> int32_t(unusedBits);
        } else {
            constexpr uint32_t mask = uint32_t(0xFFFFFFFFu) >> unusedBits;
            value &= mask;
            if (value & (uint32_t(1) << (N-1))) {
                value |= ~mask;
            }
            return int32_t(value);
        }
    }


    int32_t m_x;
    int32_t m_z;
    int32_t m_y;
};

class packet_writer {
public:
    packet_writer() = default;

    void write_varint(int32_t);
    void write_bytes(std::span<const std::byte>);
    void write_bool(bool);
    void write_u8(uint8_t);
    void write_u16(uint16_t);
    void write_u32(uint32_t);
    void write_u64(uint64_t);
    void write_i8(int8_t);
    void write_i16(int16_t);
    void write_i32(int32_t);
    void write_i64(int64_t);

    void write_identifier(std::string_view str) { write_string<32767>(str); }

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
