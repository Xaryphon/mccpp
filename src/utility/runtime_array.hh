#pragma once

#include <memory>

namespace mccpp::utility {

template<typename T>
class runtime_array {
public:
    runtime_array()
    : m_data(nullptr), m_size(0)
    {}

    explicit runtime_array(size_t size)
    : m_data(std::make_unique<T[]>(size)), m_size(size)
    {}

    runtime_array(std::unique_ptr<T[]> &&data, size_t size)
    : m_data(data), m_size(size)
    {}

    runtime_array(runtime_array<T> &&other)
    {
        m_data = std::move(other.m_data);
        m_size = other.m_size;
        other.m_size = 0;
    }

    ~runtime_array()
    {}

    runtime_array<T> &operator=(runtime_array<T> &&other)
    {
        m_data = std::move(other.m_data);
        m_size = other.m_size;
        other.m_size = 0;
        return *this;
    }

    T *data()
    {
        return m_data.get();
    }

    bool empty()
    {
        return data();
    }

    T &operator[](size_t pos)
    {
        return data()[pos];
    }

    size_t size()
    {
        return m_size;
    }

    T *release()
    {
        return m_data.release();
    }

    void reset(T *ptr, size_t size) noexcept
    {
        m_data.reset(ptr);
        m_size = size;
    }

private:
    std::unique_ptr<T[]> m_data;
    size_t m_size;
};

}
