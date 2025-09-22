#include "AppState.h"

AppState::AppState()
{
    std::error_code ec;
    auto path = fs::path(RUNTIME_ASSETS_DIR);
    auto it = fs::directory_iterator(path, fs::directory_options::skip_permission_denied, ec);
    if (ec) {
        std::println(stderr, "Can't read asset dir: {}", path.string());
        return;
    }
    for (; it != fs::directory_iterator(); ++it) {
        const auto& de = *it;
        if (de.is_regular_file(ec) && !de.path().filename().string().starts_with('.')) {
            builtin_scene_filenames.push_back(de.path().filename().string());
        }
    }
    ra::sort(builtin_scene_filenames);
}
