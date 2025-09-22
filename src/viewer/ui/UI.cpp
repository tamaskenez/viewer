#include "UI.h"

#include "AppState.h"
#include "SceneToRender.h"

#include "util/Camera.h"
#include "util/sdl_util.h"

#include <meadow/cppext.h>

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_video.h>

#include <imgui.h>

#include <format>

class UIImpl : public UI
{
public:
    const AppState& app_state;

    struct UIState {
        bool show_demo_window = false;
    } ui_state;

    UIImpl(const AppState& app_state_arg)
        : app_state(app_state_arg)
    {
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
            ImGui::Text("Built-in scenes");
            ImGui::Separator();
            std::vector<const char*> items;
            for (size_t i : vi::iota(0u, app_state.builtin_scene_names.size())) {
                items.push_back(app_state.builtin_scene_names[i].c_str());
            }
            int current_item = -1;
            if (app_state.current_prebuilt_scene_ix) {
                current_item = iicast<int>(*app_state.current_prebuilt_scene_ix);
            }
            ImGui::ListBox("Click to load", &current_item, items.data(), iicast<int>(items.size()));
            ImGui::Separator();

            static std::string text = "<no value>";
            if (ImGui::Button("GetClipboardText")) {
                text = app_state.get_clipboard();
            }
            ImGui::Text("%s", std::format("from clipboard: {}", text).c_str());

            // load custom model
            // change light pitch
            // change light yaw
            // change light color
            // change background color
            // post http request with description
            // reset view

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

std::unique_ptr<UI> UI::make(const AppState& app_state)
{
    return std::make_unique<UIImpl>(app_state);
}
