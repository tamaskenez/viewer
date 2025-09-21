#pragma once

#include "SceneToRender.h"
#include "util/Camera.h"

#include <optional>

struct AppState {
    Camera camera;
    std::optional<SceneToRender> str;
};
