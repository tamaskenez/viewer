#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <meadow/matlab.h>

const float k_default_fovy = matlab::deg2rad(45.0f);

struct Camera {
    glm::vec3 pos = glm::vec3(0, 0, -1);
    glm::vec3 lookat = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
    float fovy = k_default_fovy; // [rad]
    float near = 0.01f;
    float far = 100.0f;
};

glm::mat4 make_view_projection_matrix(const Camera& c, float aspect_ratio);
Camera make_camera_for_bounding_box(const std::array<glm::vec3, 2>& bounding_box);
