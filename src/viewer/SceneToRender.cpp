#include "SceneToRender.h"

#include <glm/gtc/type_ptr.hpp>

MeshToRender::MeshToRender(size_t material_ix_arg)
    : material_ix(material_ix_arg)
    , vertex_array(gl_gen_vertex_array())
    , vbo_pos(0)
    , vbo_norm(0)
    , ebo(0)
{
}

SceneToRender::SceneToRender(
  std::string glsl_version_arg,
  const std::array<glm::vec3, 2>& bounding_box_arg,
  std::vector<Material> materials_arg,
  std::vector<MeshToRender> meshes_arg
)
    : glsl_version(MOVE(glsl_version_arg))
    , bounding_box(bounding_box_arg)
    , materials(MOVE(materials_arg))
    , meshes(MOVE(meshes_arg))
{
    const char vertex_shader_source[] = R"--(
        layout(location = 0) in vec4 vPosition;
        uniform mat4 uMVP;
        void main()
        {
           gl_Position = uMVP * vPosition;
        })--";

    const char fragment_shader_source[] = R"--(
        precision mediump float;
        out vec4 fragColor;
        void main()
        {
           fragColor = vec4(1, 0, 0, 1);
        })--";

    program = gl_create_attach_link_program(
      gl_compile_shader_source(GL_VERTEX_SHADER, std::format("{}\n{}", glsl_version, vertex_shader_source)),
      gl_compile_shader_source(GL_FRAGMENT_SHADER, std::format("{}\n{}", glsl_version, fragment_shader_source))
    );
}

void SceneToRender::render(const glm::mat4& mvp)
{
    CHECK_GL_VOID(glUseProgram(*program));
    auto mvp_loc = CHECK_GL(glGetUniformLocation(*program, "uMVP"));
    CHECK_GL_VOID(glUniformMatrix4fv(mvp_loc, 1, false, glm::value_ptr(mvp)));
    for (auto& mtr : meshes) {
        mtr.render();
    }
}

void MeshToRender::render()
{
    CHECK_GL_VOID(glBindVertexArray(*vertex_array));
    CHECK_GL_VOID(glDrawElements(GL_TRIANGLES, iicast<GLsizei>(ebo_index_count), GL_UNSIGNED_INT, nullptr));
}
