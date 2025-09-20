#include "Renderer.h"

#include "util/gl_util.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

Renderer::Renderer()
{
#ifndef __EMSCRIPTEN__
    gladLoaderLoadGL();
#endif

    CHECK_GL(glBindBuffer(GL_TEXTURE_2D, 1));

    const float vertices[] = {
      0, // v0
      0,
      0,
      1, // v1
      0,
      0,
      0, // v2
      1,
      0,
      0, // v3
      0,
      0,
      0, // v4
      1,
      0,
      1, // v5
      0,
      0
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const unsigned indices[] = {0, 1, 2, 3, 4, 5};
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    glBindVertexArray(0);

    // TODO: compile simple vertex/fragment shaders → cubeShader
}

Renderer::~Renderer()
{
#ifndef __EMSCRIPTEN__
    gladLoaderUnloadGL();
#endif
}

void Renderer::start_3d_scene(const SDL_Rect& viewport, const SDL_FColor& clear_color)
{
    // Rendering
    glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
// Draw your 3D scene
DrawCube(display_w, display_h);

#endif
}

void Renderer::draw_scene() {}

void Renderer::reset_state_before_imgui_overlay()
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);
    glBindVertexArray(0);
}
