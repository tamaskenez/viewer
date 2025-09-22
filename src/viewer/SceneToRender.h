#pragma once

#include "util/gl_util.h"

#include <string>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

struct Camera;

struct MeshToRender {
    size_t material_ix;                                // Set in constructor.
    gl_unique_name<GLName::vertex_array> vertex_array; // Generated in constructor.

    gl_unique_name<GLName::buffer> vbo_pos, vbo_norm, ebo;
    size_t ebo_index_count = 0;

    explicit MeshToRender(size_t material_ix);
    void render();
};

struct Material {
    struct Color {
        std::optional<glm::vec4> ambient, diffuse, specular;
    } color;
    std::optional<float> shininess, shininess_strength;
};

// Preprocessed scene, ready to be rendered.
struct SceneToRender {
public:
    SceneToRender(
      std::string glsl_version,
      const std::array<glm::vec3, 2>& bounding_box,
      std::vector<Material> materials,
      std::vector<MeshToRender> meshes
    );

    void render(const Camera& camera, float aspect_ratio, const glm::vec3& light_dir);

    const std::array<glm::vec3, 2>& get_bounding_box() const
    {
        return bounding_box;
    }

private:
    std::string glsl_version;
    std::array<glm::vec3, 2> bounding_box;
    std::vector<Material> materials;
    std::vector<MeshToRender> meshes;
    gl_unique_name<GLName::program> program = gl_unique_name<GLName::program>(0);
};
