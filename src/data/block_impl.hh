#pragma once

#include <cstdint>
#include <cstddef>

namespace mccpp::data {

using state_id = uint16_t;
using block_id = uint16_t;
using property_id = uint8_t;

namespace impl::property {
    // bits 0-4 count
    //      5-6 type
    //          0 enum
    //          1 bool
    //          2 int 0
    //          3 int 1
    //        7 unused
    //      8-f value offset (for enums)
    extern const uint16_t flags[];
    extern const char *const name[];
    extern const char *const key[];
    extern const char *const values[];
}

namespace impl::block {
    extern const char *const name[];
    extern const char *const translation_key[];
    extern const state_id first_state_id[];
    extern const state_id default_state_id[];
    // bits 0-a offset
    //      b-d count
    //      e-f unused
    extern const uint16_t properties[];
    extern const property_id property_map[];
}

namespace impl::state {
    extern const block_id block[];
}

}
