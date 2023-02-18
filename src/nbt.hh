#pragma once

#include "proto/packet.hh"

namespace mccpp::nbt {

enum tag_type {
    TAG_END,
    TAG_BYTE,
    TAG_SHORT,
    TAG_INT,
    TAG_LONG,
    TAG_FLOAT,
    TAG_DOUBLE,
    TAG_BYTE_ARRAY,
    TAG_STRING,
    TAG_LIST,
    TAG_COMPOUND,
    TAG_INT_ARRAY,
    TAG_LONG_ARRAY,
};

std::string_view tag_type_to_string(tag_type type);

class parse_error : public std::runtime_error {
public:
    parse_error(const std::string &what_arg)
    : std::runtime_error(what_arg)
    {}

    parse_error(const char *what_arg)
    : std::runtime_error(what_arg)
    {}
};

class tag_byte;
class tag_short;
class tag_int;
class tag_long;
class tag_float;
class tag_double;
class tag_byte_array;
class tag_string;
class tag_list;
class tag_compound;
class tag_int_array;
class tag_long_array;

class tag {
public:
    virtual ~tag() = default;
    virtual tag_type type() = 0;

    static std::unique_ptr<tag> create(tag_type, proto::packet_reader &);

    tag_byte &as_byte();
    tag_short &as_short();
    tag_int &as_int();
    tag_long &as_long();
    tag_float &as_float();
    tag_double &as_double();
    tag_byte_array &as_byte_array();
    tag_string &as_string();
    tag_list &as_list();
    tag_compound &as_compound();
    tag_int_array &as_int_array();
    tag_long_array &as_long_array();
};

class tag_byte final : public tag {
public:
    tag_type type() override { return TAG_BYTE; }
    tag_byte(proto::packet_reader &);

    int8_t &value() { return m_value; }

private:
    int8_t m_value;
};

class tag_short final : public tag {
public:
    tag_type type() override { return TAG_SHORT; }
    tag_short(proto::packet_reader &);

    int16_t &value() { return m_value; }

private:
    int16_t m_value;
};

class tag_int final : public tag {
public:
    tag_type type() override { return TAG_INT; }
    tag_int(proto::packet_reader &);

    int32_t &value() { return m_value; }

private:
    int32_t m_value;
};

class tag_long final : public tag {
public:
    tag_type type() override { return TAG_LONG; }
    tag_long(proto::packet_reader &);

    int64_t &value() { return m_value; }

private:
    int64_t m_value;
};

class tag_float final : public tag {
public:
    tag_type type() override { return TAG_FLOAT; }
    tag_float(proto::packet_reader &);

    float &value() { return m_value; }

private:
    float m_value;
};

class tag_double final : public tag {
public:
    tag_type type() override { return TAG_DOUBLE; }
    tag_double(proto::packet_reader &);

    double &value() { return m_value; }

private:
    double m_value;
};

class tag_byte_array final : public tag {
public:
    tag_type type() override { return TAG_BYTE_ARRAY; }
    tag_byte_array(proto::packet_reader &);
};

class tag_string final : public tag {
public:
    tag_type type() override { return TAG_STRING; }
    tag_string(proto::packet_reader &);

    std::string &value() {
        return m_value;
    }

private:
    std::string m_value;
};

class tag_list final : public tag {
public:
    tag_type type() override { return TAG_LIST; }
    tag_list(proto::packet_reader &);

    using element_type = std::unique_ptr<tag>;
    using container_type = std::vector<element_type>;
    using iterator = container_type::iterator;

    iterator begin() { return m_container.begin(); }
    iterator end() { return m_container.end(); }

    tag_type item_type();
    bool item_type_compatible(tag_type);

private:
    container_type m_container;
};

class tag_compound final : public tag {
public:
    tag_type type() override { return TAG_COMPOUND; }
    tag_compound(proto::packet_reader &);

    using element_type = std::tuple<std::string, std::unique_ptr<tag>>;
    using container_type = std::vector<element_type>;
    using iterator = container_type::iterator;

    iterator begin() { return m_container.begin(); }
    iterator end() { return m_container.end(); }

private:
    container_type m_container;
};

class tag_int_array final : public tag {
public:
    tag_type type() override { return TAG_INT_ARRAY; }
    tag_int_array(proto::packet_reader &);

    using element_type = int32_t;
    using container_type = std::vector<element_type>;
    using iterator = container_type::iterator;

    iterator begin() { return m_container.begin(); }
    iterator end() { return m_container.end(); }

private:
    container_type m_container;
};

class tag_long_array final : public tag {
public:
    tag_type type() override { return TAG_LONG_ARRAY; }
    tag_long_array(proto::packet_reader &);

    using element_type = int64_t;
    using container_type = std::vector<element_type>;
    using iterator = container_type::iterator;

    iterator begin() { return m_container.begin(); }
    iterator end() { return m_container.end(); }

private:
    container_type m_container;
};

class nbt {
public:
    explicit nbt(mccpp::proto::packet_reader &);

    tag &root() { return *m_root; }

private:
    std::unique_ptr<tag> m_root;
};

void dump_nbt(mccpp::proto::packet_reader &s);

}
