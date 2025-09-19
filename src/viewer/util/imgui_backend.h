#pragma once

#include "util/sdl_util.h"

#include <imgui.h>

// sdl/opengl/imgui-related display and rendering controller.
class ImGuiBackend
{
    sdl_unique_ptr<SDL_Window> window;

public:
    ImGuiBackend();

    void begin_frame();
    void end_frame(const ImVec4& clear_color);
};
