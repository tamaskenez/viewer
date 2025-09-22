#include "App.h"

#include "AppState.h"
#include "import3d.h"

#include "ui/UI.h"
#include "util/SDLApp.h"
#include "util/UserInputTo3DNavigation.h"
#include "util/imgui_backend.h"
#include "util/sdl_util.h"

#include <meadow/cppext.h>

#include <imgui_impl_sdl3.h>

#ifdef __EMSCRIPTEN__
  #include "util/emscripten_browser_clipboard.h"

namespace
{
void browser_paste_handler(std::string&& paste_data, void* app_state)
{
    std::println("browser_paste_handler called");
    reinterpret_cast<AppState*>(app_state)->set_clipboard_text_pasted_into_browser(MOVE(paste_data));
}
} // namespace
#endif

struct App : public SDLApp {
    SDLEventLogger event_logger;
    AppState app_state;
    ImGuiBackend imgui_backend;
    std::unique_ptr<UI> ui;
    UserInputTo3DNavigation user_input_to_3d_navigation;

    App(int /*argc*/, char** /*argv*/)
        : ui(UI::make(app_state))
    {
        //        load_scene(fs::path(RUNTIME_ASSETS_DIR) / "single_brick.zip");
        //        load_scene(fs::path(RUNTIME_ASSETS_DIR) / "steampunk_airship.zip");
        load_scene(fs::path(RUNTIME_ASSETS_DIR) / "brick2.zip");

#ifdef __EMSCRIPTEN__
        emscripten_browser_clipboard::paste(&browser_paste_handler, &app_state);
#endif
    }

    SDL_AppResult SDL_AppIterate() override
    {
        // TODO: throttle refresh when no change.
        imgui_backend.begin_frame();

        int w, h;
        CHECK_SDL(SDL_GetWindowSizeInPixels(imgui_backend.get_sdl_window(), &w, &h));
        const float aspect_ratio = float(w) / float(h);

        if (app_state.str) {
            app_state.str->render(app_state.camera, aspect_ratio);
        }

        ui->render_imgui_content();

        imgui_backend.end_frame();

        return SDL_APP_CONTINUE;
    }

    SDL_AppResult SDL_AppEvent(SDL_Event* event) override
    {
        const auto timestamp = double(reinterpret_cast<SDL_CommonEvent*>(event)->timestamp) / 1e9;
        // std::println("{:.3f} {}", timestamp, sdl_get_event_description(event));
        ImGui_ImplSDL3_ProcessEvent(event);
        const auto& io = ImGui::GetIO();
        const auto min_window_size = std::min(io.DisplaySize.x, io.DisplaySize.y);
        const bool skip_processing = is_sdl_mouse_event(event->type) && io.WantCaptureMouse;
        if (!skip_processing) {
            if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                return SDL_APP_SUCCESS;
            } else if (auto* mouse_motion = sdl_event_cast<SDL_MouseMotionEvent>(event)) {
                if (user_input_to_3d_navigation.is_mouse_captured()) {
                    user_input_to_3d_navigation.mouse_moved_rel(
                      mouse_motion->xrel / min_window_size, mouse_motion->yrel / min_window_size, app_state.camera
                    );
                }
            } else if (auto* mouse_button = sdl_event_cast<SDL_MouseButtonEvent>(event)) {
                if (mouse_button->down) {
                    user_input_to_3d_navigation.mouse_button_down(mouse_button->button);
                } else {
                    user_input_to_3d_navigation.mouse_button_up(mouse_button->button);
                }
            } else if (event->type == SDL_EVENT_MOUSE_WHEEL) {
                auto* mouse_wheel = reinterpret_cast<SDL_MouseWheelEvent*>(event);
                user_input_to_3d_navigation.wheel(mouse_wheel->x, mouse_wheel->y, app_state.camera);
            }
        }
        return SDL_APP_CONTINUE;
    }

    void SDL_AppQuit(SDL_AppResult /*result*/) override {}

    void load_scene(const fs::path& path)
    {
        app_state.str = import_to_scene_to_render(path, imgui_backend.get_glsl_version());
        auto bb = app_state.str->get_bounding_box();
        const auto bb_center = (bb[0] + bb[1]) / 2.0f;
        const auto diameter = glm::length(bb[1] - bb[0]);
        const auto fovy = k_default_fovy;
        const auto viewing_distance = diameter / sin(fovy);
        app_state.camera = Camera{
          .pos = bb_center - glm::vec3(0, 0, viewing_distance),
          .lookat = bb_center,
          .up = glm::vec3(0, 1, 0),
          .fovy = fovy,
          .near = diameter / 100.0f,
          .far = diameter * 100.0f
        };
    }
};

std::unique_ptr<SDLApp> make_app(int argc, char** argv)
{
    return std::make_unique<App>(argc, argv);
}
