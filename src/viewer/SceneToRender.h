#pragma once

#include "util/gl_util.h"

#include <string>

#include <glm/mat4x4.hpp>

// Preprocessed scene, ready to be rendered.
class SceneToRender
{
public:
    explicit SceneToRender(std::string glsl_version_arg);
    void render(const glm::mat4& mvp);

private:
    std::string glsl_version;
    GLVertexArrayWithBuffers vaobs;
    gl_unique_name<GLName::program> program = gl_unique_name<GLName::program>(0);
};
