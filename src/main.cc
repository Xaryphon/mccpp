#include <iostream>

#include "application.hh"
#include "data/block.hh"
#include <vector>
#include <algorithm>
#include "identifier.hh"
#include "logger.hh"
#include "resource/block_state.hh"
#include "vfs/vfs.hh"
#include "vfs/host.hh"

using variant_kv_pair = std::pair<std::string_view, std::string_view>;

std::vector<variant_kv_pair> parse_variant_kv_pairs(std::string_view variant) {
    std::vector<variant_kv_pair> list {};
    while (!variant.empty()) {
        std::string_view match = variant.substr(0, variant.find(','));
        size_t equals_pos = match.find('=');
        assert(equals_pos != std::string_view::npos);
        std::string_view key = match.substr(0, equals_pos);
        std::string_view value = match.substr(equals_pos + 1);
        list.emplace_back(std::move(key), std::move(value));
        if (match.size() < variant.size())
            variant.remove_prefix(match.size() + 1);
        else
            variant = {};
    }
    return list;
}

unsigned parse_uint(std::string_view str) {
    unsigned v = 0;
    for (char c : str) {
        if (c < '0' || c > '9')
            throw std::invalid_argument("str does not represent a valid unsigned integer");
        v = v * 10 + (c - '0');
    }
    return v;
}

bool match_block_state(mccpp::data::state state, const std::vector<variant_kv_pair> &variant) {
    using namespace mccpp;
    for (auto prop_value : data::state(state)) {
        auto prop = prop_value.property();
        auto iter = std::find_if(variant.begin(), variant.end(), [prop](const variant_kv_pair &kv_pair) -> bool {
            return kv_pair.first == prop.key();
        });
        if (iter == variant.end()) {
            continue;
        }
        if (prop.is_enum()) {
            if (iter->second != prop[prop_value.value()]) {
                return false;
            }
        } else if (prop.is_bool()) {
            if (iter->second != (prop_value.value() ? "true" : "false")) {
                return false;
            }
        } else {
            assert(prop.is_int());
            unsigned value = parse_uint(iter->second);
            if (value != prop.is_int1() + prop_value.value()) {
                return false;
            }
        }
    }
    return true;
}

void match_block_states(mccpp::data::block block, const std::vector<variant_kv_pair> &variant) {
    using namespace mccpp;
    for (data::state_id state = block.first_state_id(); data::impl::state::block[state] == block.id(); ++state) {
        if (match_block_state(data::state(state), variant)) {
            MCCPP_I("   {}", data::state(state).id());
        }
    }
}

int main(int argc, char **argv) {
    // Clear the terminal to work around VSCode/CodeLLDB bullshit
    // FIXME: Figure out an alternative solution for this
    std::clog << "\033[3J" << std::flush;
    std::clog.sync_with_stdio(false);

    using namespace mccpp;

    vfs::vfs fs {};
    fs.storages().emplace_back(std::make_unique<vfs::host_storage>("assets"));
    fs.build_tree();

    MCCPP_I("block count : {}", data::impl::block::count);
    MCCPP_I("state count : {}", data::impl::state::count);

    data::block block(28);
    MCCPP_I("{} {}", block.id(), block.name());

    identifier block_id(block.name());

    const vfs::tree_node *rbs_file = fs.find_file(fmt::format("{}/blockstates/{}.json", block_id.name_space(), block_id.name()));
    if (rbs_file == nullptr) {
        return 1;
    }
    resource::block_state rbs(fs, *rbs_file);

    for (resource::block_state_variant &variant : rbs) {
        MCCPP_I("{}", variant.match);
        match_block_states(block, parse_variant_kv_pairs(variant.match));
    }

    //return 0;
    return mccpp::main(argc, argv);
}
