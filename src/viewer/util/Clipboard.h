#pragma once

#include <functional>
#include <string>

// Clipboard provides access to the clipboard text. If emscripten it notifies when the user pasted to the browser
// clipboard (on desktop the app detects Cmd/Ctrl+V).

namespace detail
{
void browser_paste_handler(const std::string& paste_data, void* that);
}

class Clipboard
{
public:
    explicit Clipboard(std::function<void()> on_browser_paste_fn);
    std::string get_text() const;

private:
    std::function<void()> on_browser_paste_fn;
    std::string clipboard_text_pasted_into_browser;

    friend void detail::browser_paste_handler(const std::string&, void*);
};
