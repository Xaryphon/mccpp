#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mccpp {

inline uint64_t extract_bits(const std::vector<uint64_t> &data, size_t offset, size_t bits) {
    uint64_t fl_pos = offset / 64;
    uint64_t fl_off = offset % 64;
    uint64_t fl_num = bits > 64 - fl_off ? 64 - fl_off : bits;
    uint64_t fl_mask = (1 << fl_num) - 1;
    uint64_t fl = (data[fl_pos] >> (64 - fl_off - fl_num)) & fl_mask;
    uint64_t sl_pos = fl_pos + 1;
    uint64_t sl_num = bits - fl_num;
    uint64_t sl_mask = (1 << sl_num) - 1;
    uint64_t sl = (data[sl_pos] >> (64 - sl_num)) & sl_mask;
    uint64_t ret = fl << sl_num | sl;
    return ret;
}

}
