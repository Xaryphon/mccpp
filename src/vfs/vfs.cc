#include "vfs.hh"
#include "logger.hh"

namespace mccpp::vfs {

void vfs::build_tree() {
    for (auto &s : m_storages) {
        tree_node *current = &m_root;
        auto iter = s->create_iterator();
        for (;;) {
            if (!iter->next()) {
                current = current->parent();
                if (current == nullptr) {
                    break;
                }
            } else {
                tree_node &new_entry = current->emplace_back(std::string(iter->name()));
                if (iter->is_directory()) {
                    current = &new_entry;
                } else {
                    new_entry.storage(s.get());
                }
            }
        }
    }
}

struct split_string_iterator {
    split_string_iterator(std::string_view str)
    : part(), remaining(str)
    {
        ++*this;
    }

    std::string_view part;
    std::string_view remaining;

    std::string_view operator*() {
        return part;
    }

    split_string_iterator &operator++() {
        size_t index = remaining.find('/');
        if (index == std::string_view::npos) {
            part = remaining;
            remaining = {};
        } else {
            part = remaining.substr(0, index);
            remaining = remaining.substr(index + 1);
        }
        return *this;
    }

    bool operator==(std::nullptr_t) const {
        return part.empty() && remaining.empty();
    }
};

class split_string {
public:
    using iterator = split_string_iterator;

    split_string(std::string_view str)
    : m_str(str)
    {}

    iterator begin() {
        return { m_str };
    }
    std::nullptr_t end() { return nullptr; }

private:
    std::string_view m_str;
};

runtime_array<std::byte> vfs::read_file(std::string_view path) {
    tree_node *node = &m_root;
    for (std::string_view part : split_string(path)) {
        node = node->find(part);
        if (!node) {
            // FIXME: Handle in a better way
            MCCPP_E("Unable to find file {}", path);
            return {};
        }
    }

    if (!node->storage()) {
        // FIXME: Handle in a better way
        MCCPP_E("Found but no storage associated (probably a directory): {}", path);
        return {};
    }

    return node->storage()->read_file(path);
}

}
