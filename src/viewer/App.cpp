#include "App.h"

#include "AppState.h"
#include "Event.h"
#include "import3d.h"

#include "ui/UI.h"
#include "util/SDLApp.h"
#include "util/UserInputTo3DNavigation.h"
#include "util/http.h"
#include "util/imgui_backend.h"
#include "util/sdl_util.h"

#include <imgui_impl_sdl3.h>
#include <meadow/cppext.h>
#include <nlohmann/json.hpp>

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

namespace
{
template<glm::length_t L, typename T, glm::qualifier Q>
nlohmann::json to_json_array(const glm::vec<L, T, Q>& v)
{
    auto a = nlohmann::json::array();
    for (glm::length_t i : vi::iota(0, v.length())) {
        a.push_back(v[i]);
    }
    return a;
}
} // namespace

struct App : public SDLApp {
    AppState app_state;
    ImGuiBackend imgui_backend;
    std::unique_ptr<UI> ui;
    UserInputTo3DNavigation user_input_to_3d_navigation;

    App(int /*argc*/, char** /*argv*/)
        : ui(UI::make(app_state))
    {
        load_scene(0);

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

        if (app_state.document) {
            app_state.document->str.render(app_state.camera, aspect_ratio);
        }

        ui->render_imgui_content();

        imgui_backend.end_frame();

        return SDL_APP_CONTINUE;
    }

    SDL_AppResult SDL_AppEvent(SDL_Event* event) override
    {
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
            } else if (auto* mouse_wheel = sdl_event_cast<SDL_MouseWheelEvent>(event)) {
                user_input_to_3d_navigation.wheel(mouse_wheel->x, mouse_wheel->y, app_state.camera);
            } else if (auto* user = sdl_event_cast<SDL_UserEvent>(event)) {
                auto user_event_ptr = reinterpret_cast<std::any*>(user->data1);
                handle_user_events(user_event_ptr);
                delete user_event_ptr;
            }
        }
        return SDL_APP_CONTINUE;
    }

    void handle_user_events(std::any* event)
    {
        if (std::any_cast<Event::ResetView>(event)) {
            if (app_state.document) {
                app_state.camera = make_camera_for_bounding_box(app_state.document->str.get_bounding_box());
            }
        } else if (std::any_cast<Event::SendHttpPostRequest>(event)) {
            if (app_state.document) {
                app_state.pending_http_request = PendingHttpRequest{
                  .payload = nlohmann::json(
                    {{"filename", app_state.document->filename},
                                              {"light",
                      nlohmann::json(
                        {{"elevation", app_state.light_elevation},
                         {"declination", app_state.light_declination},
                         {"color", to_json_array(app_state.light_color)}}
                      )},
                                              {"background_color", to_json_array(app_state.background_color)}}
                  )
                };
                ui->confirm_http_post_request(std::format(
                  "Click OK to send an HTTP POST request to {}\n\nBody:\n\"{}\"",
                  k_http_endpoint,
                  app_state.pending_http_request->payload.dump(2)
                ));
            }
        } else if (auto* http_post_request_event_confirmed = std::any_cast<Event::HttpPostRequestConfirmed>(event)) {
            if (!app_state.pending_http_request) {
                std::println(stderr, "Missing pending_http_request");
                assert(false);
                return;
            }
            if (http_post_request_event_confirmed->ok) {
                send_http_post_request(
                  k_http_endpoint,
                  "application/json",
                  app_state.pending_http_request->payload.dump(),
                  [](int status_code, std::string status_text, std::string response) {
                      send_event_to_app(Event::HttpRequestCompleted{
                        .status_code = status_code, .status_text = MOVE(status_text), .response = MOVE(response)
                      });
                  }
                );
            }
            app_state.pending_http_request.reset();
        } else if (auto* http_request_completed = std::any_cast<Event::HttpRequestCompleted>(event)) {
            ui->display_message_box(std::format(
              "HTTP request completed {} ({})\nResponse:\n{}",
              http_request_completed->status_text,
              http_request_completed->status_code,
              http_request_completed->response
            ));
        } else {
            std::println(stderr, "Unhandled user event: {}", event->type().name());
            CHECK(false);
        }
    }

    void SDL_AppQuit(SDL_AppResult /*result*/) override {}

    void load_scene(size_t prebuilt_scene_ix)
    {
        CHECK(!app_state.builtin_scene_filenames.empty());
        auto filename = app_state.builtin_scene_filenames.at(prebuilt_scene_ix);
        auto path = fs::path(RUNTIME_ASSETS_DIR) / filename;
        app_state.document = Document{
          .prebuilt_scene_ix = prebuilt_scene_ix,
          .filename = filename,
          .str = import_to_scene_to_render(path, imgui_backend.get_glsl_version())
        };
        app_state.camera = make_camera_for_bounding_box(app_state.document->str.get_bounding_box());
    }
};

std::unique_ptr<SDLApp> make_app(int argc, char** argv)
{
    return std::make_unique<App>(argc, argv);
}
