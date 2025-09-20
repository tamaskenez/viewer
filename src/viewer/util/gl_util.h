#pragma once

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <glad/gl.h>
#endif

#include <source_location>

void check_gl(const std::source_location location = std::source_location::current());

#define CHECK_GL(_CALL) \
    do {                \
        _CALL;          \
        check_gl();     \
    } while (0)
