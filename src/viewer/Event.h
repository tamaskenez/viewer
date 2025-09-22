#pragma once

#include <any>
#include <deque>
#include <optional>

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
} // namespace Event

void send_event_to_app(std::any event);
uint32_t get_user_event_type();
