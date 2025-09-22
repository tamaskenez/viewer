#include "sdl_util.h"

#include <meadow/cppext.h>
#include <print>

template<>
const SDL_KeyboardEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP
           ? reinterpret_cast<const SDL_KeyboardEvent*>(event)
           : nullptr;
}

template<>
const SDL_QuitEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_QUIT ? reinterpret_cast<const SDL_QuitEvent*>(event) : nullptr;
}

template<>
const SDL_MouseMotionEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_MOUSE_MOTION ? reinterpret_cast<const SDL_MouseMotionEvent*>(event) : nullptr;
}

template<>
const SDL_MouseWheelEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_MOUSE_WHEEL ? reinterpret_cast<const SDL_MouseWheelEvent*>(event) : nullptr;
}

template<>
const SDL_MouseButtonEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP
           ? reinterpret_cast<const SDL_MouseButtonEvent*>(event)
           : nullptr;
}

template<>
const SDL_UserEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_USER ? reinterpret_cast<const SDL_UserEvent*>(event) : nullptr;
}

void print_sdl_display_info()
{
    const auto primary_display_id = CHECK_SDL(SDL_GetPrimaryDisplay());
    std::println("SDL_GetDisplayName: {}", CHECK_SDL(SDL_GetDisplayName(primary_display_id)));
    std::println("SDL_GetCurrentVideoDriver: {}", CHECK_SDL(SDL_GetCurrentVideoDriver()));
    for (int i = 0; i < SDL_GetNumVideoDrivers(); ++i) {
        std::println("- VideoDriver#{}: {}", i, SDL_GetVideoDriver(i));
    }

    int count;
    SDL_DisplayID* displays = CHECK_SDL(SDL_GetDisplays(&count));
    for (int i = 0; i < count; ++i) {
        std::println("display #{}: {}", i, displays[i]);
    }

    SDL_Rect rect;
    CHECK_SDL(SDL_GetDisplayBounds(primary_display_id, &rect));
    std::println("SDL_GetDisplayBounds: ({} {}) ({} x {})", rect.x, rect.y, rect.w, rect.h);

    CHECK_SDL(SDL_GetDisplayUsableBounds(primary_display_id, &rect));
    std::println("SDL_GetDisplayUsableBounds: ({} {}) ({} x {})", rect.x, rect.y, rect.w, rect.h);

    auto print_display_mode = [](std::string_view title, const SDL_DisplayMode* dm) {
        std::println("---- {} ----", title);
        std::println("DisplayMode::format: {}", SDL_GetPixelFormatName(dm->format));
        std::println("DisplayMode::w,h: {} x {}", dm->w, dm->h);
        std::println("DisplayMode::pixel_density: {}", dm->pixel_density);
        std::println(
          "DisplayMode::refresh_rate: {} = {}/{}",
          dm->refresh_rate,
          dm->refresh_rate_numerator,
          dm->refresh_rate_denominator
        );
    };

    auto* dm = CHECK_SDL(SDL_GetDesktopDisplayMode(primary_display_id));
    print_display_mode("Desktop display mode", dm);

    SDL_DisplayMode** display_modes = CHECK_SDL(SDL_GetFullscreenDisplayModes(primary_display_id, &count));
    for (int i = 0; i < count; ++i) {
        print_display_mode(std::format("display mode #{}", i), display_modes[i]);
    }

    std::println("SDL_GetDisplayContentScale: {}", CHECK_SDL(SDL_GetDisplayContentScale(primary_display_id)));
}

bool is_sdl_mouse_event(SDL_EventType type)
{
    static constexpr auto values = {
      SDL_EVENT_WINDOW_MOUSE_ENTER,
      SDL_EVENT_WINDOW_MOUSE_LEAVE,
      SDL_EVENT_MOUSE_MOTION,
      SDL_EVENT_MOUSE_BUTTON_DOWN,
      SDL_EVENT_MOUSE_BUTTON_UP,
      SDL_EVENT_MOUSE_WHEEL,
      SDL_EVENT_MOUSE_ADDED,
      SDL_EVENT_MOUSE_REMOVED
    };
    return ra::find(values, type) != values.end();
}

std::string sdl_get_event_description(const SDL_Event* event)
{
    const auto buf_size = SDL_GetEventDescription(event, nullptr, 0);
    std::string buf(iicast<size_t>(buf_size), 0);
    SDL_GetEventDescription(event, buf.data(), buf_size);
    return buf;
}

void SDLEventLogger::log(const SDL_Event* event)
{
    const auto* ce = reinterpret_cast<const SDL_CommonEvent*>(event);
    auto it = history.find(ce->type);
    if (it == history.end()) {
        std::println("[{:.3f}] {}", double(ce->timestamp) / 1e9, sdl_get_event_description(event));
        history.emplace(event->type, EventTypeHistory{.last_logged_at = ce->timestamp});
    } else {
        if (auto& deferred = it->second.deferred) {
            ++deferred->count;
            deferred->until = ce->timestamp;
            deferred->last_message = sdl_get_event_description(event);
        } else {
            it->second.deferred = Deferred{
              .count = 1,
              .since = ce->timestamp,
              .until = ce->timestamp,
              .last_message = sdl_get_event_description(event)
            };
        }
    }
    for (auto& [k, v] : history) {
        if (v.deferred && double(ce->timestamp - v.last_logged_at) / 1e9 > 5) {
            std::println("[{:.3f}] {}", double(v.deferred->until) / 1e9, v.deferred->last_message);
            std::println("    {} since {:.3f}", v.deferred->count, double(v.deferred->since) / 1e9);
            v.last_logged_at = ce->timestamp;
            v.deferred.reset();
        }
    }
}
