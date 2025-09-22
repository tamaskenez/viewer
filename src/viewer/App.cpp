#include "App.h"

#include "AppState.h"
#include "Event.h"
#include "import3d.h"

#include "UI.h"
#include "util/Clipboard.h"
#include "util/SDLApp.h"
#include "util/UserInputTo3DNavigation.h"
#include "util/http.h"
#include "util/imgui_backend.h"
#include "util/sdl_util.h"

#include <imgui_impl_sdl3.h>
#include <meadow/cppext.h>
#include <nlohmann/json.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

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
    Clipboard clipboard;
    UserInputTo3DNavigation user_input_to_3d_navigation;

    App(int /*argc*/, char** /*argv*/)
        : ui(UI::make(app_state))
        , clipboard([this] {
            on_clipboard_paste();
        })
    {
        send_event_to_app(Event::LoadBuiltInScene{0});
    }

    SDL_AppResult SDL_AppIterate() override
    {
        // TODO: throttle refresh when no change.
        imgui_backend.begin_frame(app_state.background_color);

        int w, h;
        CHECK_SDL(SDL_GetWindowSizeInPixels(imgui_backend.get_sdl_window(), &w, &h));
        const float aspect_ratio = float(w) / float(h);

        if (app_state.document) {
            auto light_dir = glm::rotate(glm::vec3(1, 0, 0), app_state.light_elevation, glm::vec3(0, 0, 1));
            light_dir = glm::rotate(light_dir, app_state.light_declination, glm::vec3(0, 1, 0));
            app_state.document->str.render(app_state.camera, aspect_ratio, glm::normalize(light_dir));
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
            } else if (auto* keyboard = sdl_event_cast<SDL_KeyboardEvent>(event)) {
#ifdef SDL_PLATFORM_APPLE
                constexpr auto mod = SDL_KMOD_GUI;
#else
                constexpr auto mod = SDL_KMOD_CTRL;
#endif
                if (keyboard->down && keyboard->key == SDLK_V && (keyboard->mod & mod)) {
                    on_clipboard_paste();
                }
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
                        {{"elevation", app_state.light_elevation}, {"declination", app_state.light_declination}}
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
        } else if (auto* load_built_in_scene = std::any_cast<Event::LoadBuiltInScene>(event)) {
            load_scene(load_built_in_scene->scene_ix);
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
          .str = import_to_scene_to_render_from_file(path, imgui_backend.get_glsl_version())
        };
        app_state.camera = make_camera_for_bounding_box(app_state.document->str.get_bounding_box());
    }

    void on_clipboard_paste()
    {
        std::println("App::on_clipboard_paste");
        auto clipboard_text = clipboard.get_text();
        std::println("App::on_clipboard_paste clipboard text is {} chars", clipboard_text.size());
        if (auto str = import_to_scene_to_render_from_text(clipboard_text, imgui_backend.get_glsl_version())) {
            app_state.document = Document{.prebuilt_scene_ix = std::nullopt, .filename = {}, .str = MOVE(*str)};
            app_state.camera = make_camera_for_bounding_box(app_state.document->str.get_bounding_box());
        } else {
            auto message = std::format("Failed to load scene from pasted 3D scene file, reason: {}", str.error());
            std::println("ui->display_message_box(\"{}\")", message);
            ui->display_message_box(message);
        }
    }
};

std::unique_ptr<SDLApp> make_app(int argc, char** argv)
{
    return std::make_unique<App>(argc, argv);
}
