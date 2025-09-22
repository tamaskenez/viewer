#include "UI.h"

#include "AppState.h"
#include "Event.h"
#include "SceneToRender.h"

#include "util/Camera.h"
#include "util/sdl_util.h"

#include <meadow/cppext.h>

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_video.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec4.hpp>

#include <imgui.h>

#include <format>

class UIImpl : public UI
{
public:
    AppState& app_state;

    struct UIState {
        bool show_demo_window = false;
        std::optional<std::string> confirm_http_post_request_message;
        std::optional<std::string> message_box_text;
    } ui_state;

    UIImpl(AppState& app_state_arg)
        : app_state(app_state_arg)
    {
    }

    void confirm_http_post_request(std::string message) override
    {
        ui_state.confirm_http_post_request_message = MOVE(message);
    }

    void display_message_box(std::string message) override
    {
        ui_state.message_box_text = MOVE(message);
    }

    void render_imgui_content() override
    {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
        // to learn more about Dear ImGui!).
        if (ui_state.show_demo_window) {
            ImGui::ShowDemoWindow(&ui_state.show_demo_window);
        }

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            // Scene selector.
            ImGui::Separator();
            ImGui::Text("LOAD SCENE");
            ImGui::Separator();
            std::vector<const char*> items;
            for (size_t i : vi::iota(0u, app_state.builtin_scene_filenames.size())) {
                items.push_back(app_state.builtin_scene_filenames[i].c_str());
            }
            int current_item = -1;
            if (app_state.document && app_state.document->prebuilt_scene_ix) {
                current_item = iicast<int>(*app_state.document->prebuilt_scene_ix);
            }
            ImGui::ListBox("Built-in scenes", &current_item, items.data(), iicast<int>(items.size()));
            ImGui::Text("To load a custom model,\ncopy the the file to the clipboard\nand paste it into the browser.");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("CUSTOMIZE SCENE");
            ImGui::Separator();
            ImGui::SliderAngle("Light elevation", &app_state.light_elevation, -90.0f, 90.0f, "%.0f deg", 0);
            ImGui::SliderAngle("Light declination", &app_state.light_declination, -180.0f, 180.0f, "%.0f deg", 0);

            ImGui::ColorEdit4(
              "Scene light",
              glm::value_ptr(app_state.light_color),
              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None
            );
            ImGui::ColorEdit4(
              "Background light",
              glm::value_ptr(app_state.background_color),
              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None
            );

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("HTTP REQUEST");
            ImGui::Separator();
            if (ImGui::Button("POST customization.json")) {
                send_event_to_app(Event::SendHttpPostRequest{});
            }

            if (ui_state.confirm_http_post_request_message) {
                ImGui::OpenPopup("Send customization.json");
            }

            // Always center this window when appearing
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Send customization.json", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%s", ui_state.confirm_http_post_request_message->c_str());
                bool closed = false;
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    send_event_to_app(Event::HttpPostRequestConfirmed{true});
                    ImGui::CloseCurrentPopup();
                    closed = true;
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    send_event_to_app(Event::HttpPostRequestConfirmed{false});
                    ImGui::CloseCurrentPopup();
                    closed = true;
                }
                ImGui::EndPopup();
                if (closed) {
                    ui_state.confirm_http_post_request_message.reset();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("NAVIGATION");
            ImGui::Separator();
            if (ImGui::Button("Reset view")) {
                send_event_to_app(Event::ResetView{});
            }
            ImGui::Separator();

            if (ui_state.message_box_text) {
                ImGui::OpenPopup("MessageBox");
            }
            if (ImGui::BeginPopupModal("MessageBox", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%s", ui_state.message_box_text->c_str());
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    ui_state.message_box_text.reset();
                }
                ImGui::EndPopup();
            }

#ifndef NDEBUG
            ImGui::Checkbox("Demo Window", &ui_state.show_demo_window);
            ImGui::Text(
              "Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate
            );
#endif
            ImGui::End();
        }
    }
};

std::unique_ptr<UI> UI::make(AppState& app_state)
{
    return std::make_unique<UIImpl>(app_state);
}
