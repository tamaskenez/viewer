#include "Renderer.h"

#include "util/gl_util.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <glm/gtc/matrix_transform.hpp>

#include <string_view>

Renderer::Renderer(std::string glsl_version_arg)
    : glsl_version(MOVE(glsl_version_arg))
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

glm::mat4 Renderer::start_3d_scene(const SDL_Rect& viewport, const SDL_FColor& clear_color)
{
    // Rendering
    CHECK_GL_VOID(glViewport(viewport.x, viewport.y, viewport.w, viewport.h));
    CHECK_GL_VOID(glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a));
    CHECK_GL_VOID(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    CHECK_GL_VOID(glEnable(GL_DEPTH_TEST));

    auto view = glm::lookAt(camera_pos, lookat_pos, camera_up);
    auto projection = glm::perspective(45.0f, float(viewport.w) / viewport.h, 0.1f, 100.0f);
    return projection * view;
}

void Renderer::reset_state_before_imgui_overlay()
{
    CHECK_GL_VOID(glDisable(GL_DEPTH_TEST));
    CHECK_GL_VOID(glUseProgram(0));
    CHECK_GL_VOID(glBindVertexArray(0));
}
