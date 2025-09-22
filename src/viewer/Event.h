#pragma once

#include <any>
#include <deque>
#include <optional>
#include <string>

// Events (commands) that can be sent to the App class, mostly from the UI.
namespace Event
{
struct LoadBuiltInScene {
    size_t scene_ix;
};
struct LoadCustomModelFromClipboard {
};
struct SendHttpPostRequest {
};
struct ResetView {
};
struct HttpPostRequestConfirmed {
    bool ok;
};
struct HttpRequestCompleted {
    int status_code;
    std::string status_text;
    std::string response;
};
} // namespace Event

void send_event_to_app(std::any event);
uint32_t get_user_event_type();
