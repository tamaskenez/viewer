#include "SceneToRender.h"

SceneToRender::SceneToRender(std::string glsl_version_arg)
    : glsl_version(MOVE(glsl_version_arg))
    , vaobs(2)
{
    const float vertices[] = {
      -1, // v0
      0,
      0,
      1, // v1
      0,
      0,
      0, // v2
      0.1f,
      0,
      0, // v3
      -1,
      0,
      0, // v4
      1,
      0,
      0.1f, // v5
      0,
      0
    };

    CHECK_GL_VOID(glBindVertexArray(*vaobs.vertex_array));

    CHECK_GL_VOID(glBindBuffer(GL_ARRAY_BUFFER, *vaobs.buffers[0]));
    CHECK_GL_VOID(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
    CHECK_GL_VOID(glEnableVertexAttribArray(0));
    CHECK_GL_VOID(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));

    const GLuint indices[] = {0, 1, 2, 3, 4, 5};
    CHECK_GL_VOID(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *vaobs.buffers[1]));
    CHECK_GL_VOID(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    CHECK_GL_VOID(glBindVertexArray(0));

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
    CHECK_GL_VOID(glUniformMatrix4fv(mvp_loc, 1, false, &mvp[0][0]));

    CHECK_GL_VOID(glBindVertexArray(*vaobs.vertex_array));
    CHECK_GL_VOID(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}
