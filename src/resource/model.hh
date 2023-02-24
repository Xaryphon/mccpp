#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

#include "resource.hh"
#include "texture.hh"

namespace mccpp::resource {

enum class model_cullface {
    ALWAYS,
    NEVER,
    DOWN,
    UP,
    NORTH,
    SOUTH,
    WEST,
    EAST,
};

struct model_face {
    std::optional<glm::vec4> uv;
    std::string texture;
    model_cullface cullface = model_cullface::NEVER;
    int rotation = 0;
    int tintindex = -1;
};

struct model_element {
    glm::vec3 from;
    glm::vec3 to;
    // FIXME: rotation
    bool shade = true;
    model_face down;
    model_face up;
    model_face north;
    model_face south;
    model_face west;
    model_face east;
};

class model_object final : public resource {
public:
    model_object(manager &, const identifier &, load_flags);

    void debug_dump();

    bool ambient_occlusion() const;

    texture resolve_texture(const std::string &) const;

    handle<model_object> parent();
    model_object *get_element_array();

    bool has_elements() const {
        // NOTE: This maybe incorrect since a child might be able to
        // override parent elements with an empty array
        return !m_elements.empty();
    }

    const model_element *begin() const { return m_elements.data(); }
    const model_element *end() const { return m_elements.data() + m_elements.size(); }

private:
    handle<model_object> m_parent = nullptr;
    bool m_ambient_occlusion = true;
    std::unordered_map<std::string, std::string> m_textures;
    std::vector<model_element> m_elements;
};

using model = handle<model_object>;

}
