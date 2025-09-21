#pragma once

#include "util/sdl_util.h"

#include <imgui.h>

// sdl/opengl/imgui-related display and rendering controller.
class ImGuiBackend
{
    sdl_unique_ptr<SDL_Window> window;
    std::string glsl_version;

public:
    ImGuiBackend();

    void begin_frame();
    void end_frame();

    const std::string& get_glsl_version() const
    {
        return glsl_version;
    }
    SDL_Window* get_sdl_window() const
    {
        return window.get();
    }
};
