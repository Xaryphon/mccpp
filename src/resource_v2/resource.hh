#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

#include "data/block.hh"
#include "identifier.hh"
#include "vfs/vfs.hh"

namespace mccpp::resource2 {

struct texture;

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
    glm::vec4 uv;
    const texture *texture;
    model_cullface cullface;
    float rotation;
};

struct model_element {
    glm::vec3 from;
    glm::vec3 to;
    glm::vec3 rot_origin;
    glm::vec3 rot_axis;
    float rot_angle;
    bool rot_rescale;
    bool shade;
    model_face faces[6];
};

struct model {
    bool ambient_occlusion;
    std::vector<model_element> elements;
};

class model_container {
public:
    struct iterator;

    iterator begin() const;
    iterator end() const;

    const model &operator[](const identifier &);

private:
    std::unordered_map<identifier, std::unique_ptr<model>> m_data;

    friend class manager;
};

struct state {
    const model *model;
    glm::vec2 rotation;
    bool uvlock;
};

class state_container {
public:
    using value_type = const state;
    using reference = const state &;
    using iterator = std::vector<state>::const_iterator;

    iterator begin() const;
    iterator end() const;

    reference operator[](data::state_id) const;

private:
    std::vector<state> m_data;

    friend class manager;
};

class manager {
public:
    void load(const vfs::vfs &);

    const model_container &models() { return m_models; }
    const state_container &states() { return m_states; }

private:
    model_container m_models;
    state_container m_states;
};

}
