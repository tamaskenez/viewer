#pragma once

#include "SceneToRender.h"
#include "util/Camera.h"

#include <meadow/matlab.h>

#include <glm/vec4.hpp>

#include <optional>
#include <string>
#include <vector>

struct AppState {
    Camera camera;
    std::optional<SceneToRender> str;
    std::vector<std::string> builtin_scene_names;
    std::optional<size_t> current_prebuilt_scene_ix;
    float light_elevation = matlab::deg2rad(45.0f);
    float light_declination = matlab::deg2rad(45.0f);
    glm::vec4 light_color = glm::vec4(1, 1, 1, 1);
    glm::vec4 background_color = glm::vec4(0.3, 0.3, 0.3, 1);

    AppState();
    std::string get_clipboard() const;

#ifdef __EMSCRIPTEN__
    void set_clipboard_text_pasted_into_browser(std::string clipboard_text);

private:
    std::string clipboard_text_pasted_into_browser;
#endif
};
