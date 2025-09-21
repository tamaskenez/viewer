#pragma once

#include "SceneToRender.h"
#include "util/Camera.h"

struct AppState {
    Camera camera;
    std::unique_ptr<SceneToRender> str;
};
