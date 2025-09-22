#pragma once

#include "SceneToRender.h"
#include "util/Camera.h"

#include <meadow/matlab.h>

#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

struct Document {
    std::optional<size_t> prebuilt_scene_ix; // nullopt if custom model.
    std::string filename;
    SceneToRender str;
};

struct PendingHttpRequest {
    nlohmann::json payload;
};

struct AppState {
    Camera camera;
    std::optional<Document> document;
    std::vector<std::string> builtin_scene_filenames;

    float light_elevation = matlab::deg2rad(40.0f);
    float light_declination = matlab::deg2rad(-78.0f);
    glm::vec4 background_color = glm::vec4(0.2, 0.3, 0.15, 1);

    std::optional<PendingHttpRequest> pending_http_request;

    AppState();
};
