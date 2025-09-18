#pragma once

#include <memory>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>

// Interface for handling the SDL application callbacks.
class SDLApp
{
public:
    virtual ~SDLApp() = default;

    virtual SDL_AppResult SDL_AppIterate() = 0;
    virtual SDL_AppResult SDL_AppEvent(SDL_Event* event) = 0;
    virtual void SDL_AppQuit(SDL_AppResult result) = 0;
};
