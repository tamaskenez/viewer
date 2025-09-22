#include "Event.h"

#include <meadow/cppext.h>

#include "util/sdl_util.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>

namespace
{
std::optional<uint32_t> s_user_event_type;
} // namespace

void send_event_to_app(std::any event)
{
    SDL_Event sdl_event;
    SDL_zero(sdl_event);
    sdl_event.user.type = get_user_event_type();
    sdl_event.user.timestamp = SDL_GetTicksNS();
    sdl_event.user.data1 = new std::any(MOVE(event));
    SDL_PushEvent(&sdl_event);
}

uint32_t get_user_event_type()
{
    if (!s_user_event_type) {
        s_user_event_type = CHECK_SDL(SDL_RegisterEvents(1));
    }
    return *s_user_event_type;
}
