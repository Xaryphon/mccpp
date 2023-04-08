#pragma once

#include <cassert>

#include "block_impl.hh"

namespace mccpp::data {

class property {
    enum : uint16_t {
        COUNT_MASK = 0x000f,
        TYPE_ENUM = 0x0000,
        TYPE_BOOL = 0x0020,
        TYPE_INT0 = 0x0040,
        TYPE_INT1 = 0x0060,
        TYPE_IS_INT_MASK = 0x0040,
        TYPE_MASK = 0x0060,
        OFFSET_MASK = 0xff00,
        OFFSET_SHIFT = 8,
    };

public:
    property() = default;
    property(property_id id) : m_id(id) {};

    bool is_enum() { return (flags() & TYPE_MASK) == TYPE_ENUM; }
    bool is_bool() { return (flags() & TYPE_MASK) == TYPE_BOOL; }
    bool is_int0() { return (flags() & TYPE_MASK) == TYPE_INT0; }
    bool is_int1() { return (flags() & TYPE_MASK) == TYPE_INT1; }
    bool is_int()  { return flags() & TYPE_IS_INT_MASK; }

    size_t count() { return flags() & COUNT_MASK; }

    const char *name() const { return impl::property::name[m_id]; }
    const char *key() const { return impl::property::key[m_id]; }

    const char *const *begin() {
        assert(is_enum());
        return impl::property::values + values_offset();
    }

    const char *const *end() {
        return begin() + count();
    }

    const char *operator[](size_t i) {
        return begin()[i];
    }

private:
    uint16_t flags() { return impl::property::flags[m_id]; }
    size_t values_offset() { return (flags() & OFFSET_MASK) >> OFFSET_SHIFT; }

    property_id m_id;
};

class property_value {
public:
    property_value(class property prop, size_t value)
    : m_property(prop)
    , m_value(value)
    {}

    class property property() const { return m_property; }
    size_t value() const { return m_value; }

private:
    class property m_property;
    size_t m_value;
};

class block_property_iterator {
public:
    block_property_iterator(uint16_t index) : m_index(index) {}

    property operator*() { return { map() }; }

    bool operator==(const block_property_iterator &other) const {
        return m_index == other.m_index;
    }

    block_property_iterator &operator++() {
        ++m_index;
        return *this;
    }

private:
    property_id map() { return impl::block::property_map[m_index]; }

    uint16_t m_index;
};

class block_property_view {
public:
    using iterator = block_property_iterator;

    block_property_view(block_id id) : m_id(id) {}

    iterator begin() { return { properties_start() }; }
    iterator end() {
        // Honestly... WHO THE FUCK THOUGHT THAT (UNSIGNED + UNSIGNED) CAN BE SIGNED
        return { static_cast<uint16_t>(properties_start() + properties_count()) };
    }

private:
    uint16_t properties_data() { return impl::block::properties[m_id]; }
    uint16_t properties_start() { return properties_data() & 0x7ff; }
    uint16_t properties_count() { return properties_data() >> 11; }

    block_id m_id;
};

class block {
public:
    block() = default;
    block(block_id id) : m_id(id) {}

    block_id id() { return m_id; }
    const char *name() { return impl::block::name[m_id]; }
    const char *translation_key() { return impl::block::translation_key[m_id]; }
    state_id first_state_id() { return impl::block::first_state_id[m_id]; }
    state_id default_state() { return impl::block::default_state_id[m_id]; }

    block_property_view properties() { return { m_id }; }

private:
    block_id m_id;
};

class block_state_property_iterator {
public:
    block_state_property_iterator(state_id value, block_property_iterator prop)
    : m_value(value)
    , m_prop(prop)
    {}

    block_state_property_iterator &operator++() {
        m_value /= (*m_prop).count();
        ++m_prop;
        return *this;
    }

    property_value operator*() {
        return { *m_prop, m_value % (*m_prop).count() };
    }

    bool operator==(const block_state_property_iterator &other) const {
        return m_prop == other.m_prop;
    }

private:
    state_id m_value;
    block_property_iterator m_prop;
};

class state {
public:
    state() = default;
    state(state_id id) : m_id(id) {}

    state_id id() { return m_id; }
    class block block() { return impl::state::block[m_id]; }

    block_state_property_iterator begin() {
        // once again this idiotic behaviour of u16 + u16 -> int
        return { static_cast<state_id>(m_id - block().first_state_id()), block().properties().begin() };
    }
    block_state_property_iterator end() { return { 0, block().properties().end() }; }

private:
    state_id m_id;
};

}
