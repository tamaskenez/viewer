#include "imgui_backend.h"

#include "sdl_util.h"
#include "util/gl_util.h"

#include <meadow/cppext.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <SDL3/SDL_hints.h>
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
  #if 0
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG)
    ); // Always required on Mac
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2));
  #else
    // GL 4.1 Core + GLSL 410
    const char* glsl_version = "#version 410";
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG)
    ); // Always required on Mac
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1));
  #endif
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

#ifndef __EMSCRIPTEN__
    gladLoaderLoadGL();
#endif

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

    CHECK_SDL(SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#none"));

    glsl_version = set_sdl_gl_glsl_version();
    window = init_sdl_window();
    init_imgui();

    // Setup Platform/Renderer backends
    CHECK(ImGui_ImplSDL3_InitForOpenGL(window.get(), CHECK_SDL(SDL_GL_GetCurrentContext())));
    CHECK(ImGui_ImplOpenGL3_Init(glsl_version.c_str()));

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the
    // imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    ImGui::GetIO().IniFilename = nullptr;
#endif
}

ImGuiBackend::~ImGuiBackend()
{
#ifndef __EMSCRIPTEN__
    gladLoaderUnloadGL();
#endif
}

void ImGuiBackend::begin_frame(const glm::vec4& clear_color)
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int w, h;
    CHECK_SDL(SDL_GetWindowSizeInPixels(window.get(), &w, &h));

    CHECK_GL_VOID(glViewport(0, 0, w, h));
    CHECK_GL_VOID(glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a));
    CHECK_GL_VOID(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    CHECK_GL_VOID(glEnable(GL_DEPTH_TEST));
}

void ImGuiBackend::end_frame()
{
    // Reset state before imgui overlay.
    CHECK_GL_VOID(glDisable(GL_DEPTH_TEST));
    CHECK_GL_VOID(glUseProgram(0));
    CHECK_GL_VOID(glBindVertexArray(0));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    CHECK_SDL(SDL_GL_SwapWindow(window.get()));
}
