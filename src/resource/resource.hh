#pragma once

#include <unordered_map>
#include <list>
#include <memory>
#include <typeinfo>
#include <typeindex>

#include <fmt/format.h>

#include "../identifier.hh"
#include "../utility/runtime_array.hh"
#include "application.hh"
#include "vfs/vfs.hh"

namespace mccpp::resource {

class manager;

class resource {
public:
    virtual ~resource() = default;
    virtual void finalize(manager &) {}
};

template<typename T>
using handle = T *;

class model_object;
using model = handle<model_object>;

enum class load_flags {
    RESERVED = 0,
};

template<typename T>
class container {
public:
    container(manager &mgr) : m_manager(mgr) {}

    // empty identifier returns a not found sentinel value
    handle<T> operator[](const identifier &);

private:
    manager &m_manager;
};

class manager {
public:
    manager(application &app)
    : m_assets(app.assets())
    {}

    void load();

    container<model_object> models() { return *this; }

    template<typename T>
    handle<T> get(const identifier &id, load_flags flags = {}) {
        return static_cast<T *>(get_internal(std::type_index(typeid(T)), id, flags, &manager::create_resource<T>));
    }

    runtime_array<std::byte> read_file(std::string_view);

    template<typename... Args>
    runtime_array<std::byte> read_file(fmt::format_string<Args...> format, const Args &...args) {
        std::string path = fmt::vformat(format, fmt::make_format_args(args...));
        return read_file(path);
    }

private:
    template<typename T>
    std::unique_ptr<resource> create_resource(const identifier &id, load_flags flags) {
        return std::make_unique<T>(*this, id, flags);
    }

    using create_resource_fn = std::unique_ptr<resource> (manager::*)(const identifier &, load_flags);
    using storage_key_type = std::pair<std::type_index, identifier>;
    struct storage_key_hash;
    using storage_type = std::unordered_map<storage_key_type, std::unique_ptr<resource>, storage_key_hash>;

    resource *get_internal(std::type_index, const identifier &, load_flags, create_resource_fn);

    struct storage_key_hash {
        size_t operator()(const storage_key_type &value) const {
            size_t seed = std::hash<std::type_index>()(value.first);
            seed ^= std::hash<std::string_view>()(value.second.full()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    vfs::vfs &m_assets;
    storage_type m_resources;
    std::list<std::pair<std::type_index, const identifier &>> m_init_list;

    template<typename T>
    friend class container;
};

template<typename T>
handle<T> container<T>::operator[](const identifier &id) {
    if (id.empty()) {
        // FIXME: remove the const_cast
        return &const_cast<T &>(T::not_found_sentinel);
    }
    T *res = static_cast<T*>(m_manager.get_internal(std::type_index(typeid(T)), id, load_flags::RESERVED, nullptr));
    return res ? res : &const_cast<T &>(T::not_found_sentinel);
}

template<>
[[deprecated("Use models() instead")]]
handle<model_object> manager::get<model_object>(const identifier &id, load_flags flags);

}
