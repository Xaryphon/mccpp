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

void packet_writer::write_u16(uint16_t value) {
    write_as_bytes(std::span<uint16_t>(&value, 1), true);
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

std::byte packet_reader::read_byte() {
    assert(m_remaining > 0);
    m_remaining--;
    return m_read_byte();
}

int32_t packet_reader::read_varint() {
    return varint::read([this] { return read_byte(); });
}

int64_t packet_reader::read_i64() {
    return read_int_n<int64_t>();
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

    std::string s;
    s.reserve(length);
    for (; length > 0; length--) {
        s.append(1, char(read_byte()));
    }
    return s;
}

}
