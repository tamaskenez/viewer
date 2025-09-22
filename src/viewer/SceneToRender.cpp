#include "SceneToRender.h"

#include "util/Camera.h"

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
        precision mediump float;

        layout(location = 0) in vec4 model_pos;
        layout(location = 1) in vec3 model_normal;
        
        uniform mat4 view_projection;
        
        out vec3 world_normal;
        out vec3 world_pos;
        
        void main()
        {
           world_pos = model_pos.xyz;
           world_normal = model_normal;
           gl_Position = view_projection * model_pos;
        })--";

    const char fragment_shader_source[] = R"--(
        precision mediump float;

        in vec3 world_pos;
        in vec3 world_normal;
        
        out vec4 frag_color;
        
        uniform vec3 camera_pos_world;
        uniform vec4 diffuse_color;
        uniform vec4 specular_color;
        uniform float shininess;
        uniform float shininess_strength;
        uniform vec3 dir_to_light;

        const float k_ambient_intensity = 0.1;
        
        vec4 srgb_linear_to_gamma(vec4 c) {
            return vec4(mix(12.92 * c.rgb,
                            1.055 * pow(c.rgb, vec3(1.0 / 2.4)) - 0.055,
                            greaterThan(c.rgb, vec3(0.0031308))),
                        c.a
            );
        }
        
        vec4 srgb_gamma_to_linear(vec4 c) {
            return vec4(mix(c.rgb / 12.92,
                            pow((c.rgb + 0.055) / 1.055, vec3(2.4)),
                            greaterThan(c.rgb, vec3(0.04045))),
                        c.a
            );
        }

        void main()
        {
           vec4 diffuse_color_linear = srgb_gamma_to_linear(diffuse_color);

           vec4 ambient_component = k_ambient_intensity * diffuse_color_linear;

           vec3 normal = normalize(world_normal);
           float diffuse_factor = max(dot(normal, dir_to_light), 0.0);
           vec4 diffuse_component = diffuse_factor * diffuse_color_linear;
        
           vec4 specular_component = vec4(0);
           if (diffuse_factor > 0.0) {
               vec4 specular_color_linear = srgb_gamma_to_linear(specular_color);
               vec3 view_dir = normalize(camera_pos_world - world_pos);
               vec3 reflect_dir = reflect(-dir_to_light, normal);  
               float specular_factor = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
               specular_component = shininess_strength * specular_factor * specular_color_linear;  
           }
        
           vec4 sum_components = ambient_component + diffuse_component + specular_component;
           vec4 sum_gamma = srgb_linear_to_gamma(clamp(sum_components, vec4(0, 0, 0, 0), vec4(1, 1, 1, 1)));
           frag_color = vec4(sum_gamma.rgb, 1);
        })--";

    program = gl_create_attach_link_program(
      gl_compile_shader_source(GL_VERTEX_SHADER, std::format("{}\n{}", glsl_version, vertex_shader_source)),
      gl_compile_shader_source(GL_FRAGMENT_SHADER, std::format("{}\n{}", glsl_version, fragment_shader_source))
    );
}

void SceneToRender::render(const Camera& camera, float aspect_ratio, const glm::vec3& light_dir)
{
    const auto vp = make_view_projection_matrix(camera, aspect_ratio);

    CHECK_GL_VOID(glUseProgram(*program));

    auto loc = CHECK_GL(glGetUniformLocation(*program, "view_projection"));
    CHECK(loc >= 0);
    CHECK_GL_VOID(glUniformMatrix4fv(loc, 1, false, glm::value_ptr(vp)));

    loc = CHECK_GL(glGetUniformLocation(*program, "camera_pos_world"));
    CHECK(loc >= 0);
    CHECK_GL_VOID(glUniform3fv(loc, 1, glm::value_ptr(camera.pos)));

    loc = CHECK_GL(glGetUniformLocation(*program, "dir_to_light"));
    CHECK(loc >= 0);
    CHECK_GL_VOID(glUniform3fv(loc, 1, glm::value_ptr(light_dir)));

    int specular_color_location = CHECK_GL(glGetUniformLocation(*program, "specular_color"));
    CHECK(specular_color_location >= 0);
    int shininess_location = CHECK_GL(glGetUniformLocation(*program, "shininess"));
    CHECK(shininess_location >= 0);
    int shininess_strength_location = CHECK_GL(glGetUniformLocation(*program, "shininess_strength"));
    CHECK(shininess_strength_location >= 0);

    int diffuse_color_location = CHECK_GL(glGetUniformLocation(*program, "diffuse_color"));
    CHECK(loc >= 0);

    for (auto& mtr : meshes) {
        const auto& material = materials[mtr.material_ix];
        auto diffuse_color = material.color.diffuse.value_or(glm::vec4(0.5, 0.5, 0.5, 1));
        CHECK_GL_VOID(glUniform4fv(diffuse_color_location, 1, glm::value_ptr(diffuse_color)));

        float shininess = 0.0f, shininess_strength = 0.0f;
        glm::vec4 specular_color(1, 1, 1, 1);
        if (material.shininess || material.shininess_strength || material.color.specular) {
            shininess = material.shininess.value_or(32.0f);
            shininess_strength = material.shininess_strength.value_or(1.0f);
            specular_color = material.color.specular.value_or(glm::vec4(1, 1, 1, 1));
        }
        CHECK_GL_VOID(glUniform1f(shininess_location, shininess));
        CHECK_GL_VOID(glUniform1f(shininess_strength_location, shininess_strength));
        CHECK_GL_VOID(glUniform4fv(specular_color_location, 1, glm::value_ptr(specular_color)));

        mtr.render();
    }
}

void MeshToRender::render()
{
    CHECK_GL_VOID(glBindVertexArray(*vertex_array));
    CHECK_GL_VOID(glDrawElements(GL_TRIANGLES, iicast<GLsizei>(ebo_index_count), GL_UNSIGNED_INT, nullptr));
}
