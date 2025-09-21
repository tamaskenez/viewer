#include "UserInputTo3DNavigation.h"

#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

constexpr float k_yaw_speed_factor = 5.0f;
constexpr float k_pitch_speed_factor = 5.0f;
constexpr float k_steepest_pitch = matlab::deg2rad(80.0f);
constexpr float k_pan_speed_factor = 1;
constexpr float k_zoom_speed_factor = 0.1f;

bool UserInputTo3DNavigation::is_mouse_captured() const
{
    return captured_mouse_button_ix.has_value();
}

bool UserInputTo3DNavigation::mouse_moved_rel(float xrel, float yrel, Camera& camera)
{
    if (!captured_mouse_button_ix || (xrel == 0 && yrel == 0)) {
        return false;
    }
    switch (*captured_mouse_button_ix) {
    case 1: {
        // Orbit around lookat.
        auto view_dir = camera.lookat - camera.pos;
        const auto yaw_axis = glm::vec3(0, 1, 0);
        const auto pitch_axis = glm::normalize(glm::cross(view_dir, glm::vec3(0, 1, 0)));
        view_dir = glm::rotate(view_dir, -k_yaw_speed_factor * xrel, yaw_axis);
        view_dir = glm::rotate(view_dir, -k_pitch_speed_factor * yrel, pitch_axis);
        auto q = acos(glm::dot(glm::normalize(view_dir), glm::vec3(0, 1, 0)));
        if (abs(q - num::pi / 2) > k_steepest_pitch) {
            return false;
        }
        camera.pos = camera.lookat - view_dir;
        return true;
    }
    case 3: {
        const auto pos_to_lookat = camera.lookat - camera.pos;
        const auto right = glm::normalize(glm::cross(pos_to_lookat, camera.up));
        const auto d = (xrel * right - yrel * camera.up) * glm::length(pos_to_lookat) * k_pan_speed_factor;
        camera.pos -= d;
        camera.lookat -= d;
        return true;
    }
    }
    return false;
}

void UserInputTo3DNavigation::mouse_button_up(int ix)
{
    if (captured_mouse_button_ix == ix) {
        captured_mouse_button_ix.reset();
    }
}

void UserInputTo3DNavigation::mouse_button_down(int ix)
{
    if (captured_mouse_button_ix) {
        return;
    }
    captured_mouse_button_ix = ix;
}

bool UserInputTo3DNavigation::wheel(float /*x*/, float y, Camera& camera)
{
    if (y == 0) {
        return false;
    }
    const auto pos_to_lookat = camera.lookat - camera.pos;
    auto horizontal_forward = glm::normalize(glm::vec3(pos_to_lookat.x, 0, pos_to_lookat.z));
    const auto d = y * horizontal_forward * k_zoom_speed_factor * glm::length(pos_to_lookat);
    camera.pos += d;
    camera.lookat += d;
    return true;
}
