#pragma once

#include "vfs.hh"

namespace mccpp::vfs {

class host_storage final : public storage {
public:
    explicit host_storage(std::string &&root)
    : m_root(std::move(root))
    {
        m_root += '/';
    }

    std::unique_ptr<storage_iterator> create_iterator() const override;
    runtime_array<std::byte> read_file(std::string_view) const override;

private:
    std::string m_root;
};

}
