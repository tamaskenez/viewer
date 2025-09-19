#include "UI.h"

#include "AppState.h"

#include "util/imgui_backend.h"
#include "util/opengl_imgui_headers.h"
#include "util/sdl_util.h"

#include <meadow/cppext.h>

#include <SDL3/SDL_init.h>

class UIImpl : public UI
{
public:
    const AppState& app_state;
    ImGuiBackend imgui_backend;

    struct UIState {
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    } ui_state;

    UIImpl(const AppState& app_state_arg)
        : app_state(app_state_arg)
    {
    }

    void render_frame() override
    {
        imgui_backend.begin_frame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
        // to learn more about Dear ImGui!).
        if (ui_state.show_demo_window)
            ImGui::ShowDemoWindow(&ui_state.show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
            ImGui::Checkbox(
              "Demo Window", &ui_state.show_demo_window
            ); // Edit bools storing our window open/close ui_state
            ImGui::Checkbox("Another Window", &ui_state.show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);               // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", &ui_state.clear_color.x); // Edit 3 floats representing a color

            if (ImGui::Button("Button"
                )) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text(
              "Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate
            );
            ImGui::End();
        }

        // 3. Show another simple window.
        if (ui_state.show_another_window) {
            ImGui::Begin(
              "Another Window", &ui_state.show_another_window
            ); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool
               // when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                ui_state.show_another_window = false;
            ImGui::End();
        }

        imgui_backend.end_frame(ui_state.clear_color);
    }
};

std::unique_ptr<UI> UI::make(const AppState& app_state)
{
    return std::make_unique<UIImpl>(app_state);
}
