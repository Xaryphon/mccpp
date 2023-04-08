#include "block_state.hh"

#include <nlohmann/json.hpp>

#include "logger.hh"

namespace mccpp::resource {

block_state::block_state(vfs::vfs &fs, const vfs::tree_node &node) {
    auto data = fs.read_file(node);
    auto json = nlohmann::json::parse(data);
    assert(json.is_object());

    if (auto iter = json.find("variants"); iter != json.end()) {
        auto &variants = *iter;
        assert(variants.is_object());
        for (auto iter = variants.begin(); iter != variants.end(); ++iter) {
            auto &variant = iter.value();
            block_state_variant &v = m_variants.emplace_back();
            v.match = iter.key();
            v.flags = 0;
            if (auto iter = variant.find("x"); iter != variant.end()) {
                assert(iter->is_number_integer());
                int value = iter->get<int>();
                assert(value % 90 == 0);
                value = value / 90 % 4;
                v.flags |= value;
            }
            if (auto iter = variant.find("y"); iter != variant.end()) {
                assert(iter->is_number_integer());
                int value = iter->get<int>();
                assert(value % 90 == 0);
                value = value / 90 % 4;
                v.flags |= value << 2;
            }
            if (auto iter = variant.find("uvlock"); iter != variant.end()) {
                assert(iter->is_boolean());
                bool value = iter->get<bool>();
                if (value)
                    v.flags |= 0x10;
            }
        }
    }

    if (auto iter = json.find("multipart"); iter != json.end()) {
        MCCPP_W("Multipart block states not supported");
    }
}

}
