#pragma once

#include <functional>
#include <string>

// Thee Clipboard class provides access to either the system clipboard (native) or the text pasted into the web
// application. The web application is notified when the user pasted into the browser window (on desktop the app detects
// Cmd/Ctrl+V).

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
