#pragma once

#include "util/gl_util.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct SDL_Rect;
struct SDL_FColor;

class Renderer
{
public:
    explicit Renderer(std::string glsl_version);
    ~Renderer();

    glm::mat4 start_3d_scene(const SDL_Rect& viewport, const SDL_FColor& clear_color);
    void reset_state_before_imgui_overlay();

private:
    std::string glsl_version;
    glm::vec3 camera_pos = glm::vec3(0, 0, -10);
    glm::vec3 lookat_pos = glm::vec3(0, 0, 0);
    glm::vec3 camera_up = glm::vec3(0, 1, 0);
};
