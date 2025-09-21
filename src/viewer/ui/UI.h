#pragma once

#include <memory>

#include <meadow/matlab.h>

struct AppState;

const float k_default_fovy = matlab::deg2rad(45.0f);

class UI
{
public:
    static std::unique_ptr<UI> make(const AppState& app_state);

    virtual ~UI() = default;

    virtual void render_imgui_content() = 0;
};
