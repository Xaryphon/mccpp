#include "resource.hh"

#include <algorithm>
#include <cassert>
#include <fstream>

#include "../logger.hh"
#include "../utility/scope_guard.hh"
#include "model.hh"
#include "vfs/vfs.hh"

namespace mccpp::resource {

void manager::load() {
    for (auto &namespace_node_ptr : m_assets.root()) {
        auto &namespace_node = *namespace_node_ptr;
        if (const vfs::tree_node *models_node = namespace_node.find("models")) {
            if (const vfs::tree_node *block_node = models_node->find("block")) {
                for (auto &node_ptr : *block_node) {
                    auto &node = *node_ptr;
                    if (!node.name().ends_with(".json")) {
                        MCCPP_W("Skipping loading of unexpected file assets/{}/models/block/{}", namespace_node.name(), node.name());
                        continue;
                    }
                    auto model = std::make_unique<model_object>(m_assets, node);
                    std::string_view basename = node.name();
                    basename.remove_suffix(5);
                    identifier id = fmt::format("{}:block/{}", namespace_node.name(), basename);
                    m_resources[{ std::type_index(typeid(model_object)), std::move(id) }] = std::move(model);
                }
            }
        }
    }

    for (auto &[key, res] : m_resources) {
        (void)key;
        res->finalize(*this);
    }
}

runtime_array<std::byte> manager::read_file(std::string_view path) {
    MCCPP_T("Reading asset \"{}\"", path);
    return m_assets.read_file(path);
}

resource *manager::get_internal(std::type_index type, const identifier &id,
                   load_flags flags, create_resource_fn factory)
{
    auto iter = m_resources.find({ type, id });
    if (iter != m_resources.end())
        return iter->second.get();

    if (factory == nullptr)
        return nullptr;

    if (std::any_of(m_init_list.begin(), m_init_list.end(), [&type, &id](const auto &item) {
            return item.first == type && item.second == id;
        }))
    {
        MCCPP_E("Detected a cycle for the following resources:");
        for (auto &item : m_init_list) {
            MCCPP_E("- {} (type: {})", item.second.full(), item.first.name());
        }
        throw std::runtime_error("resource cycle detected");
    }

    auto init_iter = m_init_list.emplace(m_init_list.begin(), type, id);
    MCCPP_SCOPE_EXIT { m_init_list.erase(init_iter); };

    std::unique_ptr<resource> res = (this->*factory)(id, flags);

    auto [iter2, inserted] = m_resources.try_emplace({ type, id }, std::move(res));
    assert(inserted);
    return iter2->second.get();
}

}
