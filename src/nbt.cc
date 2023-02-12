#include "nbt.hh"

#include <iostream>

namespace mccpp::nbt {

std::string_view tag_type_to_string(tag_type type) {
    switch (type) {
    case TAG_END: return "TAG_End";
    case TAG_BYTE: return "TAG_Byte";
    case TAG_SHORT: return "TAG_Short";
    case TAG_INT: return "TAG_Int";
    case TAG_LONG: return "TAG_Long";
    case TAG_FLOAT: return "TAG_Float";
    case TAG_DOUBLE: return "TAG_Double";
    case TAG_BYTE_ARRAY: return "TAG_Byte_Array";
    case TAG_STRING: return "TAG_String";
    case TAG_LIST: return "TAG_List";
    case TAG_COMPOUND: return "TAG_Compound";
    case TAG_INT_ARRAY: return "TAG_Int_Array";
    case TAG_LONG_ARRAY: return "TAG_Long_Array";
    default: return "TAG_Unknown";
    }
}

std::unique_ptr<tag> tag::create(tag_type type, mccpp::proto::packet_reader &s) {
    switch (type) {
    case TAG_END: abort();
    case TAG_BYTE: return std::make_unique<tag_byte>(s);
    case TAG_SHORT: return std::make_unique<tag_short>(s);
    case TAG_INT: return std::make_unique<tag_int>(s);
    case TAG_LONG: return std::make_unique<tag_long>(s);
    case TAG_FLOAT: return std::make_unique<tag_float>(s);
    case TAG_DOUBLE: return std::make_unique<tag_double>(s);
    case TAG_BYTE_ARRAY: return std::make_unique<tag_byte_array>(s);
    case TAG_STRING: return std::make_unique<tag_string>(s);
    case TAG_LIST: return std::make_unique<tag_list>(s);
    case TAG_COMPOUND: return std::make_unique<tag_compound>(s);
    case TAG_INT_ARRAY: return std::make_unique<tag_int_array>(s);
    case TAG_LONG_ARRAY: return std::make_unique<tag_long_array>(s);
    default: throw parse_error("unknown type");
    }
}

tag_byte &tag::as_byte() {
    assert(type() == TAG_BYTE);
    return static_cast<tag_byte &>(*this);
}

tag_short &tag::as_short() {
    assert(type() == TAG_SHORT);
    return static_cast<tag_short &>(*this);
}

tag_int &tag::as_int() {
    assert(type() == TAG_INT);
    return static_cast<tag_int &>(*this);
}

tag_long &tag::as_long() {
    assert(type() == TAG_LONG);
    return static_cast<tag_long &>(*this);
}

tag_float &tag::as_float() {
    assert(type() == TAG_FLOAT);
    return static_cast<tag_float &>(*this);
}

tag_double &tag::as_double() {
    assert(type() == TAG_DOUBLE);
    return static_cast<tag_double &>(*this);
}

tag_byte_array &tag::as_byte_array() {
    assert(type() == TAG_BYTE_ARRAY);
    return static_cast<tag_byte_array &>(*this);
}

tag_string &tag::as_string() {
    assert(type() == TAG_STRING);
    return static_cast<tag_string &>(*this);
}

tag_list &tag::as_list() {
    assert(type() == TAG_LIST);
    return static_cast<tag_list &>(*this);
}

tag_compound &tag::as_compound() {
    assert(type() == TAG_COMPOUND);
    return static_cast<tag_compound &>(*this);
}

tag_int_array &tag::as_int_array() {
    assert(type() == TAG_INT_ARRAY);
    return static_cast<tag_int_array &>(*this);
}

tag_long_array &tag::as_long_array() {
    assert(type() == TAG_LONG_ARRAY);
    return static_cast<tag_long_array &>(*this);
}

tag_byte::tag_byte(proto::packet_reader &s) {
    m_value = s.read_i8();
}

tag_short::tag_short(proto::packet_reader &s) {
    m_value = s.read_i16();
}

tag_int::tag_int(proto::packet_reader &s) {
    m_value = s.read_i32();
}

tag_long::tag_long(proto::packet_reader &s) {
    m_value = s.read_i64();
}

tag_float::tag_float(proto::packet_reader &s) {
    m_value = s.read_float();
}

tag_double::tag_double(proto::packet_reader &s) {
    m_value = s.read_double();
}

tag_byte_array::tag_byte_array(proto::packet_reader &) {
    throw parse_error("unimplemented");
}

tag_string::tag_string(proto::packet_reader &s) {
    uint16_t name_length = s.read_u16();
    m_value = s.read_char_array(name_length);
}

tag_list::tag_list(proto::packet_reader &s) {
    auto type = tag_type(s.read_byte());
    int32_t length = s.read_i32();
    if (length <= 0)
        return;
    m_container.reserve(length);
    while (length-- > 0) {
        m_container.emplace_back(tag::create(type, s));
    }
}

tag_type tag_list::item_type() {
    return m_container.empty() ? TAG_END : m_container[0]->type();
}

bool tag_list::item_type_compatible(tag_type type) {
    return m_container.empty() ? true : m_container[0]->type() == type;
}

tag_compound::tag_compound(proto::packet_reader &s) {
    for (;;) {
        auto type = tag_type(s.read_byte());
        if (type == TAG_END)
            break;

        uint16_t name_length = s.read_u16();
        std::string name = s.read_char_array(name_length);
        m_container.emplace_back(std::move(name), tag::create(type, s));
    }
}

tag_int_array::tag_int_array(proto::packet_reader &) {
    throw parse_error("unimplemented");
}

tag_long_array::tag_long_array(proto::packet_reader &) {
    throw parse_error("unimplemented");
}

nbt::nbt(mccpp::proto::packet_reader &s) {
    auto type = tag_type(s.read_byte());
    if (type == TAG_END)
        throw parse_error("unexpected TAG_End");

    uint16_t name_length = s.read_u16();
    std::string name = s.read_char_array(name_length);
    m_root = tag::create(type, s);
}

void dump_nbt(tag &tag, size_t indent_count = 0) {
    std::string indent(indent_count*2, ' ');
    std::string inner_indent((indent_count + 1)*2, ' ');
    fmt::print("{} ", tag_type_to_string(tag.type()));
    switch (tag.type()) {
    case TAG_BYTE: fmt::print("{}\n", tag.as_byte().value()); break;
    case TAG_SHORT: fmt::print("{}\n", tag.as_short().value()); break;
    case TAG_INT: fmt::print("{}\n", tag.as_int().value()); break;
    case TAG_LONG: fmt::print("{}\n", tag.as_long().value()); break;
    case TAG_FLOAT: fmt::print("{}\n", tag.as_float().value()); break;
    case TAG_DOUBLE: fmt::print("{}\n", tag.as_double().value()); break;
    case TAG_BYTE_ARRAY: fmt::print("TODO\n"); break;
    case TAG_STRING: fmt::print("\"{}\"\n", tag.as_string().value());break;
    case TAG_LIST:
        fmt::print("{{\n");
        for (auto &child : tag.as_list()) {
            fmt::print("{}", inner_indent);
            dump_nbt(*child, indent_count + 1);
        }
        fmt::print("{}}}\n", indent);
        break;
    case TAG_COMPOUND:
        fmt::print("{{\n");
        for (auto &[name, child] : tag.as_compound()) {
            fmt::print("{}\"{}\" ", inner_indent, name);
            dump_nbt(*child, indent_count + 1);
        }
        fmt::print("{}}}\n", indent);
        break;
    case TAG_INT_ARRAY: fmt::print("TODO\n"); break;
    case TAG_LONG_ARRAY: fmt::print("TODO\n"); break;
    default: abort();
    }
}

void dump_nbt(proto::packet_reader &s) {
    nbt file(s);
    dump_nbt(file.root());
}

}
