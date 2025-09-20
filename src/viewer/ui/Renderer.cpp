#include "Renderer.h"

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <glad/gl.h>
#endif

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

Renderer::Renderer()
{
#ifndef __EMSCRIPTEN__
    gladLoaderLoadGL();
#endif
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
