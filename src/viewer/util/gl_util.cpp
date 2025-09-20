#include "gl_util.h"

#include <cassert>
#include <exception>
#include <print>

namespace
{
std::string gl_error_string(GLenum e)
{
    switch (e) {
#define CASE(X) \
    case X:     \
        return #X;
        CASE(GL_NO_ERROR)
        CASE(GL_INVALID_ENUM)
        CASE(GL_INVALID_VALUE)
        CASE(GL_INVALID_OPERATION)
        CASE(GL_INVALID_FRAMEBUFFER_OPERATION)
        CASE(GL_OUT_OF_MEMORY)
#undef CASE
    default:
        break;
    }
    return std::format("gl_error_code({})", e);
}
} // namespace

void check_gl(const std::source_location location)
{
    if (const auto gl_error = glGetError()) {
        std::println(
          stderr,
          "{}:{}:{} gl_error: {}",
          location.file_name(),
          location.line(),
          location.column(),
          gl_error_string(gl_error)
        );
        assert(false);
        std::terminate();
    }
}

#define CHECK_GL(_CALL) \
    do {                \
        _CALL;          \
        check_gl();     \
    } while (0)
