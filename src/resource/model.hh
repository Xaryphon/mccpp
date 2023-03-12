#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

#include "resource.hh"
#include "texture.hh"
#include "vfs/vfs.hh"

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

struct model_load_data {
    identifier parent;
    std::vector<model_element> elements;
};

class model_object final : public resource {
public:
    static const model_object not_found_sentinel;

    model_object(vfs::vfs &, vfs::tree_node &);
    void finalize(manager &) override;
    bool finalized();

    void debug_dump();

    bool ambient_occlusion() const;

    const model_element *begin() const { return m_elements.data(); }
    const model_element *end() const { return m_elements.data() + m_elements.size(); }

private:
    model_object();

    std::unique_ptr<model_load_data> m_load_data;
    bool m_ambient_occlusion = true;
    std::vector<model_element> m_elements;
};

using model = handle<model_object>;

}
