#pragma once

#include <memory>
#include <vector>

#include "utility/runtime_array.hh"

namespace mccpp::vfs {

class storage_iterator {
public:
    virtual ~storage_iterator() = default;

    virtual bool is_directory() = 0;
    virtual std::string_view name() = 0;

    virtual bool next() = 0;
};

class storage {
public:
    virtual ~storage() = default;
    virtual std::unique_ptr<storage_iterator> create_iterator() = 0;
    virtual runtime_array<std::byte> read_file(std::string_view) = 0;
};

class tree_node {
public:
    tree_node(std::nullptr_t)
    : m_parent(nullptr)
    {}

    tree_node(class tree_node *parent, std::string &&name)
    : m_parent(parent)
    , m_name(std::move(name))
    {}

    class storage *storage() { return m_storage; }
    void storage(class storage *s) { m_storage = s; }

    std::string_view name() { return m_name; }
    [[nodiscard]]
    std::string generate_path() const;
    tree_node *parent() { return m_parent; }

    auto begin() { return m_entries.cbegin(); }
    auto end() { return m_entries.cend(); }

    tree_node *find(std::string_view name) {
        for (auto &entry : m_entries) {
            if (entry->name() == name)
                return entry.get();
        }
        return nullptr;
    }

    class tree_node &emplace_back(std::string &&name) {
        return *m_entries.emplace_back(std::make_unique<tree_node>(this, std::move(name)));
    }

private:
    class tree_node *const m_parent;
    const std::string m_name;
    class storage *m_storage = nullptr;
    std::vector<std::unique_ptr<tree_node>> m_entries;
};

class vfs {
public:
    std::vector<std::unique_ptr<storage>> &storages() { return m_storages; }
    void build_tree();

    tree_node &root() { return m_root; }

    runtime_array<std::byte> read_file(std::string_view);
    runtime_array<std::byte> read_file(const tree_node &);

private:
    std::vector<std::unique_ptr<storage>> m_storages;
    tree_node m_root = nullptr;
};

}
