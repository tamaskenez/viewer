#include "imgui_backend.h"

#include "opengl_imgui_headers.h"
#include "sdl_util.h"

#include <meadow/cppext.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

namespace
{
const char* set_sdl_gl_glsl_version()
{
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0));
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0));
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG)
    ); // Always required on Mac
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2));
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0));
#endif

    // Create window with graphics context
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8));

    return glsl_version;
}

sdl_unique_ptr<SDL_Window> init_sdl_window()
{
    const auto primary_display_id = CHECK_SDL(SDL_GetPrimaryDisplay());
    const float scale = CHECK_SDL(SDL_GetDisplayContentScale(primary_display_id));
    const SDL_WindowFlags window_flags =
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    auto window = sdl_unique_ptr<SDL_Window>(
      CHECK_SDL(SDL_CreateWindow("Viewer", iround<int>(1280 * scale), iround<int>(800 * scale), window_flags))
    );
    SDL_GLContext gl_context = CHECK_SDL(SDL_GL_CreateContext(window.get()));

    CHECK_SDL(SDL_GL_MakeCurrent(window.get(), gl_context));
    CHECK_SDL(SDL_GL_SetSwapInterval(1)); // Enable vsync
#ifndef __EMSCRIPTEN__
    CHECK_SDL(SDL_SetWindowPosition(window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED));
#endif
    CHECK_SDL(SDL_ShowWindow(window.get()));
    return window;
}

void init_imgui()
{
    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    CHECK(ImGui::CreateContext());
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    const float scale = CHECK_SDL(SDL_GetDisplayContentScale(CHECK_SDL(SDL_GetPrimaryDisplay())));
    style.ScaleAllSizes(scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling,
    // changing this requires resetting Style + calling this again)
    style.FontScaleDpi = scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
    // unnecessary. We leave both here for documentation purpose)
}

} // namespace

ImGuiBackend::ImGuiBackend()
{
    CHECK_SDL(SDL_Init(SDL_INIT_VIDEO));

    const char* const glsl_version = set_sdl_gl_glsl_version();
    window = init_sdl_window();
    init_imgui();

    // Setup Platform/Renderer backends
    CHECK(ImGui_ImplSDL3_InitForOpenGL(window.get(), CHECK_SDL(SDL_GL_GetCurrentContext())));
    CHECK(ImGui_ImplOpenGL3_Init(glsl_version));

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the
    // imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    ImGui::GetIO().IniFilename = nullptr;
#endif
}

void ImGuiBackend::begin_frame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiBackend::end_frame(const ImVec4& clear_color)
{
    // Rendering
    ImGui::Render();
    glViewport(0, 0, iround<int>(ImGui::GetIO().DisplaySize.x), iround<int>(ImGui::GetIO().DisplaySize.y));
    glClearColor(
      clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w
    );
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window.get());
}
