#include "UI.h"

#include "AppState.h"

#include "util/opengl_imgui_headers.h"
#include "util/sdl3_opengl3_imgui.h"
#include "util/sdl_util.h"

#include <meadow/cppext.h>

#include <SDL3/SDL_init.h>

class UIImpl : public UI
{
public:
    const AppState& app_state;
    sdl_unique_ptr<SDL_Window> window;

    struct UIState {
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    } ui_state;

    UIImpl(const AppState& app_state_arg, sdl_unique_ptr<SDL_Window> window_arg)
        : app_state(app_state_arg)
        , window(MOVE(window_arg))
    {
    }

    void render_frame() override
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

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

        // Rendering
        ImGui::Render();
        glViewport(0, 0, iround<int>(ImGui::GetIO().DisplaySize.x), iround<int>(ImGui::GetIO().DisplaySize.y));
        glClearColor(
          ui_state.clear_color.x * ui_state.clear_color.w,
          ui_state.clear_color.y * ui_state.clear_color.w,
          ui_state.clear_color.z * ui_state.clear_color.w,
          ui_state.clear_color.w
        );
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window.get());
    }
};

std::unique_ptr<UI> UI::make(const AppState& app_state)
{
    CHECK_SDL(SDL_Init(SDL_INIT_VIDEO));

    print_sdl_display_info();

    const char* const glsl_version = init_sdl_opengl();

    // Initialize SDL Window.

    const auto primary_display_id = CHECK_SDL(SDL_GetPrimaryDisplay());
    const float main_scale = CHECK_SDL(SDL_GetDisplayContentScale(primary_display_id));
    const SDL_WindowFlags window_flags =
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    auto window = sdl_unique_ptr<SDL_Window>(
      CHECK_SDL(SDL_CreateWindow("Viewer", iround<int>(1280 * main_scale), iround<int>(800 * main_scale), window_flags))
    );
    SDL_GLContext gl_context = CHECK_SDL(SDL_GL_CreateContext(window.get()));

    CHECK_SDL(SDL_GL_MakeCurrent(window.get(), gl_context));
    CHECK_SDL(SDL_GL_SetSwapInterval(1)); // Enable vsync
#ifndef __EMSCRIPTEN__
    CHECK_SDL(SDL_SetWindowPosition(window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED));
#endif
    CHECK_SDL(SDL_ShowWindow(window.get()));

    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    CHECK(ImGui::CreateContext());
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling,
    // changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
    // unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    CHECK(ImGui_ImplSDL3_InitForOpenGL(window.get(), gl_context));
    CHECK(ImGui_ImplOpenGL3_Init(glsl_version));

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the
    // imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    ImGui::GetIO().IniFilename = nullptr;
#endif

    return std::make_unique<UIImpl>(app_state, MOVE(window));
}
