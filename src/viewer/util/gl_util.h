#pragma once

#include <meadow/cppext.h>

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <glad/gl.h>
#endif

#include <source_location>
#include <type_traits>

// List of the used OpenGL name types (object types).
enum class GLName {
    shader,
    program,
    buffer,
    vertex_array
};

namespace detail
{
void delete_gl_name(GLName GLN, GLuint name);
template<class T>
T check_gl(T result, bool terminate, const std::source_location location = std::source_location::current());
void check_gl_void(bool terminate, const std::source_location location = std::source_location::current());

} // namespace detail

#define CHECK_GL_VOID(_CALL)         \
    do {                             \
        _CALL;                       \
        detail::check_gl_void(true); \
    } while (0)
#define EXPECT_GL_VOID(_CALL)         \
    do {                              \
        _CALL;                        \
        detail::check_gl_void(false); \
    } while (0)
#define CHECK_GL(_CALL) detail::check_gl(_CALL, true)
#define EXPECT_GL(_CALL) detail::check_gl(_CALL, false)

template<GLName GLN>
class gl_unique_name
{
public:
    explicit gl_unique_name(GLuint name_arg)
        : name(name_arg)
    {
    }

    gl_unique_name(const gl_unique_name&) = delete;
    gl_unique_name& operator=(const gl_unique_name&) = delete;
    gl_unique_name(gl_unique_name&& y)
        : name(y.name)
    {
        y.name = 0;
    }
    gl_unique_name& operator=(gl_unique_name&& y)
    {
        name = y.name;
        y.name = 0;
        return *this;
    }

    ~gl_unique_name()
    {
        if (name) {
            detail::delete_gl_name(GLN, name);
        }
    }
    GLuint operator*() const
    {
        CHECK(name);
        return name;
    }

private:
    GLuint name;
};

gl_unique_name<GLName::shader> gl_compile_shader_source(GLenum type, std::string_view source);

gl_unique_name<GLName::program> gl_create_attach_link_program(
  gl_unique_name<GLName::shader> vertex_shader, gl_unique_name<GLName::shader> fragment_shader
);

gl_unique_name<GLName::buffer> gl_gen_buffer();
gl_unique_name<GLName::vertex_array> gl_gen_vertex_array();

struct GLVertexArrayWithBuffers {
    gl_unique_name<GLName::vertex_array> vertex_array;
    std::vector<gl_unique_name<GLName::buffer>> buffers;

    explicit GLVertexArrayWithBuffers(size_t num_buffers);
};
