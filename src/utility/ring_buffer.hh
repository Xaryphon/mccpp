#pragma once

#include <cassert>
#include <cstddef>
#include <span>

namespace mccpp {

template<size_t N>
class ring_buffer {
public:
    ring_buffer() = default;
    ~ring_buffer() = default;

    ring_buffer(const ring_buffer &) = delete;

    size_t capacity() const {
        return N;
    }

    size_t readable() const {
        return m_written - m_read;
    }

    size_t writable() const {
        return N - readable();
    }

    std::byte front() const {
        assert(readable() >= 1);
        return m_data[m_read % N];
    }

    std::span<const std::byte> read_front() const {
        size_t start = m_read % N;
        size_t total = readable();
        size_t count = clamp(total, N - start);
        return { &m_data[start], count };
    }

    std::span<const std::byte> read_back() const {
        size_t start = m_read % N;
        size_t total = readable();
        size_t count = clamp(total, N - start);
        return { &m_data[0], total - count };
    }

    std::span<const std::byte> write_front() const {
        size_t start = m_read % N;
        size_t total = writable();
        size_t count = clamp(total, N - start);
        return { &m_data[start], count };
    }

    std::span<const std::byte> write_back() const {
        size_t start = m_read % N;
        size_t total = writable();
        size_t count = clamp(total, N - start);
        return { &m_data[0], total - count };
    }

    void erase(size_t n) {
        assert(readable() >= n);
        m_read += n;
    }

private:
    static size_t clamp(size_t v, size_t h) {
        return v > h ? h : v;
    }

    size_t m_read = 0;
    size_t m_written = 0;
    std::byte m_data[N];
};

}
