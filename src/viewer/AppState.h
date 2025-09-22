#pragma once

#include "SceneToRender.h"
#include "util/Camera.h"

#include <optional>
#include <string>
#include <vector>

struct AppState {
    Camera camera;
    std::optional<SceneToRender> str;
    std::vector<std::string> builtin_scene_names;
    std::optional<size_t> current_prebuilt_scene_ix;

    AppState();
    std::string get_clipboard() const;

#ifdef __EMSCRIPTEN__
    void set_clipboard_text_pasted_into_browser(std::string clipboard_text);

private:
    std::string clipboard_text_pasted_into_browser;
#endif
};
