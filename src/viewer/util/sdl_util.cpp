#include "sdl_util.h"
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
const SDL_MouseButtonEvent* sdl_event_cast(const SDL_Event* event)
{
    return event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP
           ? reinterpret_cast<const SDL_MouseButtonEvent*>(event)
           : nullptr;
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
