#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <meadow/matlab.h>

glm::mat4 make_view_projection_matrix(const Camera& c, float aspect_ratio)
{
    auto view = glm::lookAt(c.pos, c.lookat, c.up);
    auto projection = glm::perspective(matlab::rad2deg(c.fovy), aspect_ratio, c.near, c.far);
    return projection * view;
}
