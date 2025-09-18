#include "sdl3_opengl3_imgui.h"

#include "opengl_imgui_headers.h"

#include "util/sdl_util.h"

#include <SDL3/SDL_video.h>

const char* init_sdl_opengl()
{
    // Decide GL+GLSL versions
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
