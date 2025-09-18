#include "App.h"

#include <meadow/cppext.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

// Main entry points of the application, callbacks called by the SDL framework.

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (auto game = make_app(argc, argv)) {
        *appstate = static_cast<void*>(game.release());
        return SDL_APP_CONTINUE;
    } else {
        return SDL_APP_FAILURE;
    }
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    return reinterpret_cast<SDLApp*>(appstate)->SDL_AppIterate();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return reinterpret_cast<SDLApp*>(appstate)->SDL_AppEvent(event);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto app = std::unique_ptr<SDLApp>(reinterpret_cast<SDLApp*>(appstate));
    app->SDL_AppQuit(result);
}

#if 0 // TODO
// Main code
int main(int, char**)
{

    // Our state

    {
    }
  #ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
  #endif

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#endif
