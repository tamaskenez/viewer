#include "App.h"

#include "AppState.h"

#include "ui/UI.h"
#include "util/SDLApp.h"
#include "util/sdl_util.h"

#include <meadow/cppext.h>

#include <imgui_impl_sdl3.h>

struct App : public SDLApp {
    AppState app_state;
    std::unique_ptr<UI> ui;

    App(int /*argc*/, char** /*argv*/)
        : ui(UI::make(app_state))
    {
    }

    SDL_AppResult SDL_AppIterate() override
    {
        // TODO: throttle refresh we no change.
        ui->render_frame();
        return SDL_APP_CONTINUE;
    }

    SDL_AppResult SDL_AppEvent(SDL_Event* event) override
    {
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
        // inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
        // clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
        // clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and
        // hide them from your application based on those two flags. [If using SDL_MAIN_USE_CALLBACKS: call
        // ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]

        ImGui_ImplSDL3_ProcessEvent(event);
        if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            return SDL_APP_SUCCESS;
        }
        return SDL_APP_CONTINUE;
    }

    void SDL_AppQuit(SDL_AppResult /*result*/) override {}
};

std::unique_ptr<SDLApp> make_app(int argc, char** argv)
{
    return std::make_unique<App>(argc, argv);
}
