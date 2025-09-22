#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <meadow/matlab.h>

glm::mat4 make_view_projection_matrix(const Camera& c, float aspect_ratio)
{
    auto view = glm::lookAt(c.pos, c.lookat, c.up);
    auto projection = glm::perspective(matlab::rad2deg(c.fovy), aspect_ratio, c.near, c.far);
    return projection * view;
}

Camera make_camera_for_bounding_box(const std::array<glm::vec3, 2>& bb)
{
    const auto bb_center = (bb[0] + bb[1]) / 2.0f;
    const auto diameter = glm::length(bb[1] - bb[0]);
    const auto fovy = k_default_fovy;
    const auto viewing_distance = diameter / sin(fovy);
    return Camera{
      .pos = bb_center + glm::vec3(0, 0, viewing_distance),
      .lookat = bb_center,
      .up = glm::vec3(0, 1, 0),
      .fovy = fovy,
      .near = diameter / 100.0f,
      .far = diameter * 100.0f
    };
}
