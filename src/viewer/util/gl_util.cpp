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

void check_gl_failure(const std::source_location location, GLenum gl_error, [[maybe_unused]] bool terminate)
{
    std::println(
      stderr,
      "{}:{}:{} gl_error: {}",
      location.file_name(),
      location.line(),
      location.column(),
      gl_error_string(gl_error)
    );
    assert(false);
#ifdef NDEBUG
    if (terminate) {
        std::terminate();
    }
#endif
}
} // namespace

namespace detail
{
void check_gl_void(bool terminate, const std::source_location location)
{
    if (const auto gl_error = glGetError(); gl_error != GL_NO_ERROR) {
        check_gl_failure(location, gl_error, terminate);
    }
}

template<class T>
T check_gl(T result, bool terminate, const std::source_location location)
{
    const auto gl_error = glGetError();
    bool success = gl_error == GL_NO_ERROR;
    if constexpr (std::is_same_v<T, GLint> || std::is_same_v<T, GLuint>) {
        // assert(success == (result != T(0)));
    } else {
        static_assert(
          false,
          "The result type of the OpenGL function is expected to "
          "be GLint or GLuint."
        );
    }
    if (!success) {
        check_gl_failure(location, gl_error, terminate);
    }
    return result;
}
template GLint check_gl(GLint, bool, const std::source_location);

void delete_gl_name(GLName GLN, GLuint name)
{
    switch (GLN) {
    case GLName::shader:
        EXPECT_GL_VOID(glDeleteShader(name));
        break;
    case GLName::program:
        EXPECT_GL_VOID(glDeleteProgram(name));
        break;
    case GLName::buffer:
        EXPECT_GL_VOID(glDeleteBuffers(1, &name));
        break;
    case GLName::vertex_array:
        EXPECT_GL_VOID(glDeleteVertexArrays(1, &name));
        break;
    }
}

} // namespace detail

gl_unique_name<GLName::shader> gl_compile_shader_source(GLenum type, std::string_view source)
{
    auto shader = gl_unique_name<GLName::shader>(CHECK_GL(glCreateShader(type)));

    const GLchar* p_source = source.data();
    GLint source_length = iicast<GLint>(source.size());
    CHECK_GL_VOID(glShaderSource(*shader, 1, &p_source, &source_length));
    CHECK_GL_VOID(glCompileShader(*shader));

    GLint compile_status;
    CHECK_GL_VOID(glGetShaderiv(*shader, GL_COMPILE_STATUS, &compile_status));
    if (!compile_status) {
        GLint info_log_len = 0;
        CHECK_GL_VOID(glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &info_log_len));
        if (info_log_len) {
            std::string info_log(iicast<size_t>(info_log_len), 0);
            CHECK_GL_VOID(glGetShaderInfoLog(*shader, info_log_len, nullptr, info_log.data()));
            std::println(stderr, "Error compiling shader: {}", info_log);
            std::terminate();
        }
    }
    return shader;
}

gl_unique_name<GLName::program> gl_create_attach_link_program(
  gl_unique_name<GLName::shader> vertex_shader, gl_unique_name<GLName::shader> fragment_shader
)
{
    auto program = gl_unique_name<GLName::program>(CHECK_GL(glCreateProgram()));

    GLint shader_type;
    CHECK_GL_VOID(glGetShaderiv(*vertex_shader, GL_SHADER_TYPE, &shader_type));
    CHECK(shader_type == GL_VERTEX_SHADER);
    CHECK_GL_VOID(glGetShaderiv(*fragment_shader, GL_SHADER_TYPE, &shader_type));
    CHECK(shader_type == GL_FRAGMENT_SHADER);

    CHECK_GL_VOID(glAttachShader(*program, *vertex_shader));
    CHECK_GL_VOID(glAttachShader(*program, *fragment_shader));
    CHECK_GL_VOID(glLinkProgram(*program));

    GLint link_status;
    CHECK_GL_VOID(glGetProgramiv(*program, GL_LINK_STATUS, &link_status));
    if (!link_status) {
        GLint info_log_len = 0;
        CHECK_GL_VOID(glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &info_log_len));
        if (info_log_len) {
            std::string info_log(iicast<size_t>(info_log_len), 0);
            CHECK_GL_VOID(glGetProgramInfoLog(*program, info_log_len, nullptr, info_log.data()));
            std::println(stderr, "Error linking program: {}", info_log);
            std::terminate();
        }
    }
    return program;
}

gl_unique_name<GLName::buffer> gl_gen_buffer()
{
    GLuint name;
    CHECK_GL_VOID(glGenBuffers(1, &name));
    return gl_unique_name<GLName::buffer>(name);
}

gl_unique_name<GLName::vertex_array> gl_gen_vertex_array()
{
    GLuint name;
    CHECK_GL_VOID(glGenVertexArrays(1, &name));
    return gl_unique_name<GLName::vertex_array>(name);
}
