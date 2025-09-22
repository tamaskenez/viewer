#include "AppState.h"

#include <SDL3/SDL_clipboard.h>

AppState::AppState()
    : builtin_scene_names({"brick2.zip", "cruiser.zip", "Brachiosaurus.zip"})
{
    current_prebuilt_scene_ix = 1;
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
