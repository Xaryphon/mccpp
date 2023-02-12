#include "packet.hh"

// https://wiki.vg/Protocol

#include "varint.hh"

namespace mccpp::proto {

void packet_writer::write_varint(int32_t value) {
    varint::write(value, [this](std::byte byte) {
        m_buffer.emplace_back(byte);
    });
}

void packet_writer::write_bytes(std::span<const std::byte> data) {
    write_bytes(data, false);
}

void packet_writer::write_bool(bool value) {
    m_buffer.emplace_back(value ? std::byte(0x01) : std::byte(0x00));
}

void packet_writer::write_u8(uint8_t value) {
    m_buffer.emplace_back(std::byte(value));
}

void packet_writer::write_u16(uint16_t value) {
    write_as_bytes(std::span<uint16_t>(&value, 1), true);
}

void packet_writer::write_u32(uint32_t value) {
    write_as_bytes(std::span<uint32_t>(&value, 1), true);
}

void packet_writer::write_u64(uint64_t value) {
    write_as_bytes(std::span<uint64_t>(&value, 1), true);
}
void packet_writer::write_i8(int8_t value) {
    m_buffer.emplace_back(std::byte(value));
}

void packet_writer::write_i16(int16_t value) {
    write_as_bytes(std::span<int16_t>(&value, 1), true);
}

void packet_writer::write_i32(int32_t value) {
    write_as_bytes(std::span<int32_t>(&value, 1), true);
}

void packet_writer::write_i64(int64_t value) {
    write_as_bytes(std::span<int64_t>(&value, 1), true);
}

void packet_writer::write_bytes(std::span<const std::byte> data, bool reverse) {
    m_buffer.reserve(m_buffer.size() + data.size());
    if (reverse) {
        m_buffer.insert(m_buffer.end(), data.rbegin(), data.rend());
    } else {
        m_buffer.insert(m_buffer.end(), data.begin(), data.end());
    }
}

void packet_writer::write_string(std::string_view value, size_t max_code_points) {
    // String (n)
    // size in bytes: ≥ 1
    //                ≤ (n×4) + 3
    // UTF-8 string prefixed with its size in bytes as a VarInt.
    // Maximum length of n characters, which varies by context;
    // up to n × 4 bytes can be used to encode n characters and
    // both of those limits are checked. Maximum n value is 32767.
    // The + 3 is due to the max size of a valid length VarInt.
    assert(max_code_points <= 32767);

    // FIXME: Correctly verify that each length in code points doesn't exceed the max
    if (value.size() > max_code_points * 4) {
        throw encode_error("invalid string");
    }

    write_varint(value.size());
    write_as_bytes(std::span<const char>(value.data(), value.size()), false);
}

void packet_reader::discard(size_t n) {
    assert(m_remaining >= n);
    m_remaining -= n;
    while (n-- > 0) {
        m_read_byte();
    }
}

std::byte packet_reader::read_byte() {
    assert(m_remaining > 0);
    m_remaining--;
    return m_read_byte();
}

int32_t packet_reader::read_varint() {
    return varint::read([this] { return read_byte(); });
}

bool packet_reader::read_bool() {
    std::byte b = read_byte();
    if (b == std::byte(0x00)) {
        return false;
    } else if (b == std::byte(0x01)) {
        return true;
    } else {
        throw decode_error("invalid boolean");
    }
}

uint8_t packet_reader::read_u8() {
    return read_int_n<uint8_t>();
}

uint16_t packet_reader::read_u16() {
    return read_int_n<uint16_t>();
}

uint32_t packet_reader::read_u32() {
    return read_int_n<uint32_t>();
}

uint64_t packet_reader::read_u64() {
    return read_int_n<uint64_t>();
}

int8_t packet_reader::read_i8() {
    return read_int_n<int8_t>();
}

int16_t packet_reader::read_i16() {
    return read_int_n<int16_t>();
}

int32_t packet_reader::read_i32() {
    return read_int_n<int32_t>();
}

int64_t packet_reader::read_i64() {
    return read_int_n<int64_t>();
}

float packet_reader::read_float() {
    return read_float_n<float>();
}

double packet_reader::read_double() {
    return read_float_n<double>();
}

std::vector<std::byte> packet_reader::read_byte_array(size_t n) {
    assert(m_remaining >= n);
    m_remaining -= n;
    std::vector<std::byte> data {};
    data.reserve(n);
    while (n-- > 0)
        data.emplace_back(m_read_byte());
    return data;
}

template<typename T>
T packet_reader::read_int_n() {
    using TU = std::make_unsigned_t<T>;
    static_assert(sizeof(T) == sizeof(TU));
    TU v = 0;
    for (size_t i = std::numeric_limits<TU>::digits; i > 0; i -= 8) {
        v += TU(read_byte()) << (i - 8);
    }
    return std::bit_cast<T>(v);
}

template<typename T>
T packet_reader::read_float_n() {
    assert(m_remaining >= sizeof(T));
    m_remaining -= sizeof(T);

    std::byte bytes[sizeof(T)];
    for (size_t i = 0; i < sizeof(T); i++) {
        bytes[sizeof(T) - i - 1] = m_read_byte();
    }
    return std::bit_cast<T>(bytes);
}

std::string packet_reader::read_char_array(size_t length) {
    // FIXME: Correctly verify that each length in code points doesn't exceed the max
    std::string s;
    s.reserve(length);
    for (; length > 0; length--) {
        s.append(1, char(read_byte()));
    }
    return s;
}

std::string packet_reader::read_string(size_t max_code_points) {
    // String (n)
    // size in bytes: ≥ 1
    //                ≤ (n×4) + 3
    // UTF-8 string prefixed with its size in bytes as a VarInt.
    // Maximum length of n characters, which varies by context;
    // up to n × 4 bytes can be used to encode n characters and
    // both of those limits are checked. Maximum n value is 32767.
    // The + 3 is due to the max size of a valid length VarInt.
    assert(max_code_points <= 32767);

    // FIXME: Correctly verify that each length in code points doesn't exceed the max

    int32_t length_ = read_varint();
    if (length_ < 0) {
        throw decode_error("got a string with a negative length");
    }

    size_t length = size_t(length_);
    if (length > max_code_points) {
        throw decode_error("string length exceeds max_code_points");
    }

    return read_char_array(length);
}

}
