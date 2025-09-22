#pragma once

#include <memory>

#include <meadow/matlab.h>

struct AppState;

class UI
{
public:
    static std::unique_ptr<UI> make(AppState& app_state);

    virtual ~UI() = default;

    virtual void render_imgui_content() = 0;

    virtual void confirm_http_post_request(std::string message) = 0;
    virtual void display_message_box(std::string message) = 0;
};
