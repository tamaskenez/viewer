#pragma once

// Single file to consistently pick the right headers with the right settings.

#include <imgui.h>
#ifdef __EMSCRIPTEN__
  #define IMGUI_IMPL_OPENGL_ES3
#endif
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
  #include <SDL3/SDL_opengles2.h>
#else
  #include <SDL3/SDL_opengl.h>
#endif
