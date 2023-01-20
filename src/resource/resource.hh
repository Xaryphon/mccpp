#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>

#include "../logger.hh"
#include "../utility/runtime_array.hh"

namespace mccpp::resource {

runtime_array<std::byte> read_file(const std::string &);

template<typename TObject>
class object;

template<typename T>
class manager {
public:
    std::shared_ptr<T> find_or_create(std::string &&path) {
        std::string_view view { path };
        auto iter = m_objects.find(view);
        if (iter != m_objects.end())
            return iter->second.lock();

        auto obj = std::make_shared<T>(std::move(path));
        m_objects.emplace(std::string_view(obj->resource_path()), std::weak_ptr(obj));
        return obj;
    }

    void remove_object(object<T> *obj) {
        size_t erased = m_objects.erase(std::string_view(obj->resource_path()));
        assert(erased == 1);
        (void)erased;
    }

private:
    std::unordered_map<std::string_view, std::weak_ptr<T>> m_objects;
};

class object_base {
public:
    object_base(std::string &&path)
    : m_path(path)
    {}

    const std::string &resource_path() {
        return m_path;
    }

    bool loaded() {
        return m_loaded;
    }

    bool load(bool force_reload = false);

protected:
    virtual bool do_load(bool force_reload) = 0;

private:
    std::string m_path;
    bool m_loaded = false;
};

template<typename TObject>
class handle;

template<typename T>
class object : public object_base {
public:
    object(std::string &&path)
    : object_base(std::move(path))
    {}

    ~object() {
        s_manager().remove_object(this);
    }

private:
    static manager<T> &s_manager() {
        static manager<T> m {};
        return m;
    }

    friend handle<T>;
};

template<typename TObject>
class handle {
public:
    handle(std::string &&path)
    : m_object(TObject::s_manager().find_or_create(std::move(path)))
    {}

    TObject &operator*() {
        return *m_object;
    }

    TObject *operator->() {
        return m_object.get();
    }

private:
    std::shared_ptr<TObject> m_object;
};

}
