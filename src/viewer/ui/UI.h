#pragma once

#include <memory>

struct AppState;

class UI
{
public:
    static std::unique_ptr<UI> make(const AppState& app_state);

    virtual ~UI() = default;

    virtual void render_frame() = 0;
};
