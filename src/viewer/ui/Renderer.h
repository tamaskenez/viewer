#pragma once

struct SDL_Rect;
struct SDL_FColor;

class Renderer
{
public:
    Renderer();
    ~Renderer();
    void start_3d_scene(const SDL_Rect& viewport, const SDL_FColor& clear_color);
    void draw_scene();
    void reset_state_before_imgui_overlay();
};
