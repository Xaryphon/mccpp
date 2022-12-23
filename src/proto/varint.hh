#pragma once

#include <cassert>
#include <cstdint>
#include <bit>
#include <type_traits>
#include <limits>

#include "decode_exception.hh"

namespace mccpp::proto {

// https://wiki.vg/Protocol#VarInt_and_VarLong
namespace varint_impl {
    constexpr uint8_t SEGMENT_BITS = 0x7f;
    constexpr uint8_t CONTINUE_BIT = 0x80;

    template<typename TS, typename TCallback>
    void write_impl(TS value_, TCallback write_byte_callback) {
        static_assert(std::is_integral_v<TS>);
        static_assert(std::numeric_limits<TS>::is_signed);

        using TU = std::make_unsigned_t<TS>;
        TU value = std::bit_cast<TU>(value_);
        while (true) {
            if ((value & ~SEGMENT_BITS) == 0) {
                write_byte_callback(static_cast<std::byte>(value));
                return;
            }

            write_byte_callback(static_cast<std::byte>((value & SEGMENT_BITS) | CONTINUE_BIT));
            value >>= 7;
        }
    }

    template<typename TS, typename TCallback>
    TS read_impl(TCallback read_byte_callback) {
        static_assert(std::is_integral_v<TS>);
        static_assert(std::numeric_limits<TS>::is_signed);

        using TU = std::make_unsigned_t<TS>;
        TU value = 0;
        std::size_t position = 0;
        while (true) {
            std::byte byte_ = read_byte_callback();
            uint8_t byte = static_cast<uint8_t>(byte_);

            value |= TU(byte & SEGMENT_BITS) << position;

            if ((byte & CONTINUE_BIT) == 0)
                break;

            position += 7;

            // FIXME: Throw an exception instead
            assert(position < std::numeric_limits<TU>::digits);
            if (position >= std::numeric_limits<TU>::digits)
                throw decode_exception("invalid varint");
        }

        return std::bit_cast<TS>(value);
    }
}

namespace varint {
    template<typename T>
    void write(int32_t value, T write_byte_callback) {
        varint_impl::write_impl<int32_t>(value, write_byte_callback);
    }

    template<typename T>
    int32_t read(T read_byte_callback) {
        return varint_impl::read_impl<int32_t>(read_byte_callback);
    }
}

namespace varlong {
    template<typename T>
    void write(int64_t value, T write_byte_callback) {
        varint_impl::write_impl<int64_t>(value, write_byte_callback);
    }

    template<typename T>
    int64_t read(T read_byte_callback) {
        return varint_impl::read_impl<int64_t>(read_byte_callback);
    }
}

}
