#include "model.hh"

#include <nlohmann/json.hpp>

#include "../logger.hh"

namespace mccpp::resource {

const model_object model_object::not_found_sentinel {};

static void decode_vec3(glm::vec3 &vec, nlohmann::json &json) {
    assert(json.is_array());
    assert(json.size() == 3);
    assert(json[0].is_number());
    assert(json[1].is_number());
    assert(json[2].is_number());
    vec.x = json[0].get<float>();
    vec.y = json[1].get<float>();
    vec.z = json[2].get<float>();
}

static model_face decode_face(nlohmann::json &json) {
    model_face face {};
    assert(json.is_object());
    if (auto &uv = json["uv"]; !uv.is_null()) {
        assert(uv.is_array());
        assert(uv.size() == 4);
        assert(uv[0].is_number());
        assert(uv[1].is_number());
        assert(uv[2].is_number());
        assert(uv[3].is_number());
        face.uv.emplace(uv[0].get<float>(),
                        uv[1].get<float>(),
                        uv[2].get<float>(),
                        uv[3].get<float>());
    }
    face.texture = json["texture"].get<std::string>();
    if (auto &cullface = json["cullface"]; !cullface.is_null()) {
        const std::string &value = cullface.get<std::string>();
        if (value == "down") face.cullface = model_cullface::DOWN;
        else if (value == "up") face.cullface = model_cullface::UP;
        else if (value == "north") face.cullface = model_cullface::NORTH;
        else if (value == "south") face.cullface = model_cullface::SOUTH;
        else if (value == "west") face.cullface = model_cullface::WEST;
        else if (value == "east") face.cullface = model_cullface::EAST;
        else {
            MCCPP_W("Invalid cullface in model: \"{}\"", value);
        }
    }
    if (auto &rotation = json["rotation"]; !rotation.is_null()) {
        face.rotation = rotation.get<int>();
    }
    if (auto &tintindex = json["tintindex"]; !tintindex.is_null()) {
        face.tintindex = tintindex.get<int>();
    }
    return face;
}

static std::unique_ptr<model_load_data> load_data(nlohmann::json &json) {
    auto ptr = std::make_unique<model_load_data>();

    if (auto iter = json.find("parent"); iter != json.end()) {
        auto &parent_id = *iter;
        assert(parent_id.is_string());
        ptr->parent = parent_id.get<std::string>();
    }

    if (auto iter = json.find("display"); iter != json.end()) {
        assert(iter->is_object());
    }

    // TODO: Load textures
    //unordered_map<std::string, std::string> textures;
    //if (auto iter = json.find("textures"); iter != json.end()) {
    //    assert(iter->is_object());
    //    for (auto &[key, value] : iter->items()) {
    //        assert(value.is_string());
    //        textures[key] = value.get<std::string>();
    //    }
    //}

    if (auto iter = json.find("elements"); iter != json.end()) {
        assert(iter->is_array());
        for (auto &json : *iter) {
            assert(json.is_object());
            model_element &elem = ptr->elements.emplace_back();
            decode_vec3(elem.from, json["from"]);
            assert(elem.from.x >= -16.f && elem.from.x < 32.f);
            assert(elem.from.y >= -16.f && elem.from.y < 32.f);
            assert(elem.from.z >= -16.f && elem.from.z < 32.f);
            decode_vec3(elem.to, json["to"]);
            assert(elem.from.x >= -16.f && elem.from.x < 32.f);
            assert(elem.from.y >= -16.f && elem.from.y < 32.f);
            assert(elem.from.z >= -16.f && elem.from.z < 32.f);

            if (auto iter = json.find("shade"); iter != json.end()) {
                assert(iter->is_boolean());
                elem.shade = iter->get<bool>();
            }

            if (auto iter = json.find("faces"); iter != json.end()) {
                auto &faces = *iter;
                assert(faces.is_object());
                if (auto iter = faces.find("down");  iter != faces.end()) elem.down  = decode_face(*iter);
                else elem.down.cullface = model_cullface::ALWAYS;
                if (auto iter = faces.find("up");    iter != faces.end()) elem.up    = decode_face(*iter);
                else elem.up.cullface = model_cullface::ALWAYS;
                if (auto iter = faces.find("north"); iter != faces.end()) elem.north = decode_face(*iter);
                else elem.north.cullface = model_cullface::ALWAYS;
                if (auto iter = faces.find("south"); iter != faces.end()) elem.south = decode_face(*iter);
                else elem.south.cullface = model_cullface::ALWAYS;
                if (auto iter = faces.find("west");  iter != faces.end()) elem.west  = decode_face(*iter);
                else elem.west.cullface = model_cullface::ALWAYS;
                if (auto iter = faces.find("east");  iter != faces.end()) elem.east  = decode_face(*iter);
                else elem.east.cullface = model_cullface::ALWAYS;
            }
        }
    }

    return ptr;
}

model_object::model_object() {

}

model_object::model_object(vfs::vfs &fs, vfs::tree_node &file) {
    auto data = fs.read_file(file);
    auto json = nlohmann::json::parse(data);
    assert(json.is_object());

    m_load_data = load_data(json);

    if (auto iter = json.find("ambientocclusion"); iter != json.end()) {
        assert(iter->is_boolean());
        m_ambient_occlusion = iter->get<bool>();
    }
}

bool model_object::finalized() {
    return !bool(m_load_data);
}

void model_object::finalize(manager &mgr) {
    if (finalized())
        return;

    m_elements = std::move(m_load_data->elements);

    if (!m_load_data->parent.empty()) {
        model parent = mgr.models()[m_load_data->parent];
        if (!parent->finalized())
            parent->finalize(mgr);
        // FIXME: This is an incorrect way of checking if this model did not define elements
        // but we're probably going to make m_elements be a shared_ptr so it doesn't really matter right now
        if (m_elements.empty())
            m_elements = parent->m_elements;

        // According to https://minecraft.fandom.com/wiki/Tutorials/Models#Block_models
        // ambientocclusion: Whether to use ambient occlusion (true - default), or not (false). Note:only works on Parent file
        m_ambient_occlusion = parent->m_ambient_occlusion;
    }

    m_load_data.reset();
}

static void debug_dump_face(model_face &face) {
    if (face.uv)
        MCCPP_I("    uv: {}, {},  {}, {}", face.uv->x, face.uv->y, face.uv->z, face.uv->w);
    MCCPP_I("    texture: {}", face.texture);
    if (face.cullface != model_cullface::NEVER) {
        const char *cullface;
        switch (face.cullface) {
        case model_cullface::NEVER:
        case model_cullface::ALWAYS: abort(); // unreachable
        case model_cullface::DOWN: cullface = "down"; break;
        case model_cullface::UP: cullface = "up"; break;
        case model_cullface::NORTH: cullface = "north"; break;
        case model_cullface::SOUTH: cullface = "south"; break;
        case model_cullface::WEST: cullface = "west"; break;
        case model_cullface::EAST: cullface = "east"; break;
        }
        MCCPP_I("    cullface: {}", cullface);
    }
    MCCPP_I("    rotation: {}", face.rotation);
    //MCCPP_I("    tintindex: {}", face.tintindex);
}

void model_object::debug_dump() {
    if (this == &not_found_sentinel) {
        MCCPP_I("MODEL NOT FOUND");
    }
    MCCPP_I("Ambient Occlusion: {}", m_ambient_occlusion);
    //MCCPP_I("Textures:");
    //for (auto &[key, value] : m_textures) {
    //    MCCPP_I("  {}: {}", key, value);
    //}
    MCCPP_I("Elements:");
    for (auto &elem : m_elements) {
        MCCPP_I("- from: {}, {}, {}", elem.from.x, elem.from.y, elem.from.z);
        MCCPP_I("  to: {}, {}, {}", elem.to.x, elem.to.y, elem.to.z);
        //MCCPP_I("  shade: {}", elem.shade);
        if (elem.down.cullface  != model_cullface::ALWAYS) { MCCPP_I("  down:");  debug_dump_face(elem.down);  }
        if (elem.up.cullface    != model_cullface::ALWAYS) { MCCPP_I("  up:");    debug_dump_face(elem.up);    }
        if (elem.north.cullface != model_cullface::ALWAYS) { MCCPP_I("  north:"); debug_dump_face(elem.north); }
        if (elem.south.cullface != model_cullface::ALWAYS) { MCCPP_I("  south:"); debug_dump_face(elem.south); }
        if (elem.west.cullface  != model_cullface::ALWAYS) { MCCPP_I("  west:");  debug_dump_face(elem.west);  }
        if (elem.east.cullface  != model_cullface::ALWAYS) { MCCPP_I("  east:");  debug_dump_face(elem.east);  }
    }
}

}
