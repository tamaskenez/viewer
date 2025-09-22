#include "AppState.h"

#include <SDL3/SDL_clipboard.h>

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
}

#ifdef __EMSCRIPTEN__
std::string AppState::get_clipboard() const
{
    return clipboard_text_pasted_into_browser;
}
void AppState::set_clipboard_text_pasted_into_browser(std::string clipboard_text)
{
    clipboard_text_pasted_into_browser = MOVE(clipboard_text);
}
#else
std::string AppState::get_clipboard() const
{
    if (auto* clipboard_text = SDL_GetClipboardText()) {
        return clipboard_text;
    } else {
        return {};
    }
}
#endif
