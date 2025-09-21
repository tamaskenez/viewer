#pragma once

#include <optional>

struct Camera;

// UserInputTo3DNavigation turns user input (mouse, keys) to 3d navigation commands.
class UserInputTo3DNavigation
{
public:
    bool is_mouse_captured() const;
    // Update camera, if need. Return true if changed.
    // xrel, yrel must be normalized to min(window_width, window_height)
    bool mouse_moved_rel(float xrel, float yrel, Camera& camera);
    void mouse_button_up(int button_ix);
    void mouse_button_down(int button_ix);
    bool wheel(float x, float y, Camera& camera);

private:
    std::optional<int> captured_mouse_button_ix;
};
