#include "import3d.h"

#include "SceneToRender.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meadow/cppext.h>

#include <glm/common.hpp>
#include <glm/vec3.hpp>

namespace
{
constexpr auto k_importer_flags = aiProcessPreset_TargetRealtime_Quality | aiProcess_PreTransformVertices;

static_assert(
  std::is_same_v<ai_real, float>,
  "Implementation of import_to_scene_to_render assumes that ai_real is float but it isn't."
);

template<class T>
auto to_glm(const aiVector3t<T>& v)
{
    return glm::vec<3, T, glm::defaultp>(v.x, v.y, v.z);
}

SceneToRender import_to_scene_to_render(const aiScene* scene, std::string glsl_version)
{
    // Note that aiProcess_PreTransformVertices simplifies rendering but it's a bad idea for scenes where a single brick
    // is instantiated many times, so:
    // TODO: remove aiProcess_PreTransformVertices and implement hierarchical scene rendering.

    std::vector<Material> materials;
    materials.resize(scene->mNumMaterials);
    for (unsigned i : vi::iota(0u, scene->mNumMaterials)) {
        auto* mat = scene->mMaterials[i];
        aiColor4D c;
        if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &c) == aiReturn_SUCCESS) {
            materials[i].color.ambient = glm::vec4(c.r, c.g, c.b, c.a);
        }
        if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &c) == aiReturn_SUCCESS) {
            materials[i].color.diffuse = glm::vec4(c.r, c.g, c.b, c.a);
        }
        if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &c) == aiReturn_SUCCESS) {
            materials[i].color.specular = glm::vec4(c.r, c.g, c.b, c.a);
        }
        float f;
        if (aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &f) == aiReturn_SUCCESS) {
            materials[i].shininess = f;
        }
        if (aiGetMaterialFloat(mat, AI_MATKEY_SHININESS_STRENGTH, &f) == aiReturn_SUCCESS) {
            materials[i].shininess_strength = f;
        }
    }

    std::vector<MeshToRender> meshes;
    meshes.reserve(scene->mNumMeshes);
    std::array<glm::vec3, 2> bounding_box = {
      glm::vec3(INFINITY, INFINITY, INFINITY), glm::vec3(-INFINITY, -INFINITY, -INFINITY)
    };
    for (unsigned mesh_ix : vi::iota(0u, scene->mNumMeshes)) {
        auto* mesh = scene->mMeshes[mesh_ix];

        // mesh->mAABB is not initialized, has to calculate it manually.
        for (unsigned i : vi::iota(0u, mesh->mNumVertices)) {
            const auto v = to_glm(mesh->mVertices[i]);
            bounding_box[0] = glm::min(bounding_box[0], v);
            bounding_box[1] = glm::max(bounding_box[1], v);
        }

        auto& mtr = meshes.emplace_back(mesh->mMaterialIndex);
        CHECK_GL_VOID(glBindVertexArray(*mtr.vertex_array));

        mtr.vbo_pos = gl_gen_buffer();
        CHECK_GL_VOID(glBindBuffer(GL_ARRAY_BUFFER, *mtr.vbo_pos));
        CHECK_GL_VOID(glBufferData(
          GL_ARRAY_BUFFER, iicast<GLsizeiptr>(mesh->mNumVertices * sizeof(aiVector3D)), mesh->mVertices, GL_STATIC_DRAW
        ));
        CHECK_GL_VOID(glEnableVertexAttribArray(0));
        CHECK_GL_VOID(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

        if (mesh->HasNormals()) {
            mtr.vbo_norm = gl_gen_buffer();
            CHECK_GL_VOID(glBindBuffer(GL_ARRAY_BUFFER, *mtr.vbo_norm));
            CHECK_GL_VOID(glBufferData(
              GL_ARRAY_BUFFER,
              iicast<GLsizeiptr>(mesh->mNumVertices * sizeof(aiVector3D)),
              mesh->mNormals,
              GL_STATIC_DRAW
            ));
            CHECK_GL_VOID(glEnableVertexAttribArray(1));
            CHECK_GL_VOID(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
        }

        std::vector<GLuint> indices;
        indices.reserve(mesh->mNumFaces * 3);
        for (unsigned i : vi::iota(0u, mesh->mNumFaces)) {
            const auto& face = mesh->mFaces[i];
            for (unsigned j : vi::iota(0u, face.mNumIndices)) {
                indices.push_back(face.mIndices[j]);
            }
        }
        mtr.ebo_index_count = indices.size();

        mtr.ebo = gl_gen_buffer();
        CHECK_GL_VOID(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *mtr.ebo));
        CHECK_GL_VOID(glBufferData(
          GL_ELEMENT_ARRAY_BUFFER, iicast<GLsizeiptr>(indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW
        ));
        CHECK_GL_VOID(glBindVertexArray(0));
    }
    return SceneToRender(glsl_version, bounding_box, MOVE(materials), MOVE(meshes));
}

} // namespace

SceneToRender import_to_scene_to_render_from_file(const std::filesystem::path& path, std::string glsl_version)
{
    Assimp::Importer importer;
    return import_to_scene_to_render(importer.ReadFile(path, k_importer_flags), glsl_version);
}

std::expected<SceneToRender, std::string>
import_to_scene_to_render_from_text(std::string_view text, std::string glsl_version)
{
    Assimp::Importer importer;
    try {
        std::println("About to import text data of size {}", text.size());
        auto* scene = importer.ReadFileFromMemory(text.data(), text.size(), k_importer_flags);
        if (scene) {
            std::println("Importer returned non-null scene.");
        } else {
            std::println("Importer returned error");
            std::println("...which is: {}", importer.GetErrorString());
            return std::unexpected(importer.GetErrorString());
        }
        return import_to_scene_to_render(scene, glsl_version);
    } catch (std::exception& e) {
        std::println("Importer threw exception.");
        std::println("...which is: {}", e.what());
        return std::unexpected(e.what());
    } catch (...) {
        std::println("Importer threw unknown exception.");
        return std::unexpected("Unknown exception");
    }
}
