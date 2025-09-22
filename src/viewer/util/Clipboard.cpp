#include "Clipboard.h"

#include <SDL3/SDL_clipboard.h>
#include <meadow/cppext.h>
#include <print>

#ifdef __EMSCRIPTEN__
  #include "util/emscripten_browser_clipboard.h"
#endif

#ifdef __EMSCRIPTEN__
namespace detail
{
void browser_paste_handler(const std::string& paste_data, void* that)
{
    std::println("browser_paste_handler got {} chars and that = {}", paste_data.size(), that);
    auto* clipboard = reinterpret_cast<Clipboard*>(that);
    clipboard->clipboard_text_pasted_into_browser = paste_data;
    if (clipboard->on_browser_paste_fn) {
        std::println("browser_paste_handler about to call on_browser_paste_fn");
        clipboard->on_browser_paste_fn();
        std::println("browser_paste_handler done calling on_browser_paste_fn");
    }
}
} // namespace detail

Clipboard::Clipboard(std::function<void()> on_browser_paste_fn_arg)
    : on_browser_paste_fn(MOVE(on_browser_paste_fn_arg))
{
    emscripten_browser_clipboard::paste(&detail::browser_paste_handler, this);
}

std::string Clipboard::get_text() const
{
    return clipboard_text_pasted_into_browser;
}
#else

Clipboard::Clipboard(std::function<void()>) {}

std::string Clipboard::get_text() const
{
    if (!SDL_HasClipboardText()) {
        return {};
    }
    return SDL_GetClipboardText();
}

#endif
