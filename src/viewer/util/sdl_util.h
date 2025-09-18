#pragma once

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include <cassert>
#include <exception>
#include <format>
#include <memory>
#include <source_location>

// A std::unique_ptr-like object for SDL types, like SDL_Window, SDL_Renderer,
// etc.. which calls the appropriate SDL_Destroy* function in its destructor.
template<class T>
class sdl_unique_ptr;

template<class T>
sdl_unique_ptr(T*) -> sdl_unique_ptr<T>;

#define DEFINE_SDL_UNIQUE_PTR(TYPE, DELETER_FN)                               \
    struct TYPE##_Deleter {                                                   \
        void operator()(TYPE* p) const                                        \
        {                                                                     \
            DELETER_FN(p);                                                    \
        }                                                                     \
    };                                                                        \
    template<>                                                                \
    class sdl_unique_ptr<TYPE> : public std::unique_ptr<TYPE, TYPE##_Deleter> \
    {                                                                         \
    public:                                                                   \
        sdl_unique_ptr() = default;                                           \
        explicit sdl_unique_ptr(TYPE* p)                                      \
            : std::unique_ptr<TYPE, TYPE##_Deleter>(p)                        \
        {                                                                     \
        }                                                                     \
    };

DEFINE_SDL_UNIQUE_PTR(SDL_Window, SDL_DestroyWindow)
DEFINE_SDL_UNIQUE_PTR(SDL_Renderer, SDL_DestroyRenderer)
DEFINE_SDL_UNIQUE_PTR(SDL_Surface, SDL_DestroySurface)

#undef DEFINE_SDL_UNIQUE_PTR

template<class T>
T check_sdl(T result, bool terminate_on_error, const std::source_location location = std::source_location::current())
{
    bool success;
    if constexpr (std::is_same_v<T, bool>) {
        success = result;
    } else if constexpr (std::is_pointer_v<T>) {
        success = result != nullptr;
    } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        success = result != T(0);
    } else {
        static_assert(
          false,
          "The result type of the SDL function is expected to "
          "be bool, pointer or integral."
        );
    }
    if (!success) {
        SDL_LogMessage(
          SDL_LOG_CATEGORY_APPLICATION,
          terminate_on_error ? SDL_LOG_PRIORITY_CRITICAL : SDL_LOG_PRIORITY_ERROR,
          "%s",
          std::format(
            "{}:{}:{} sdl_error: {}", location.file_name(), location.line(), location.column(), SDL_GetError()
          )
            .c_str()
        );

        if (terminate_on_error) {
            std::terminate();
        }
    }
    assert(success);
    return result;
}

#define CHECK_SDL(X) check_sdl(X, true)
#define EXPECT_SDL(X) check_sdl(X, false)

template<class T>
const T* sdl_event_cast(const SDL_Event* event);

void print_sdl_display_info();
