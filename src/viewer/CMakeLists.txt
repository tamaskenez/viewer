file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.h *.cpp *.c)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(viewer MACOSX_BUNDLE WIN32
	${sources}
)
target_compile_definitions(viewer PRIVATE
    MEADOW_NO_USING_STD
)
target_include_directories(viewer PRIVATE
	.
	glad/include
)
target_link_libraries(viewer PRIVATE
	imgui_backend_sdl3_opengl3
	meadow
	assimp::assimp
)

if(NOT EMSCRIPTEN)
	target_link_libraries(viewer PRIVATE glad-opengl_4_1)
endif()
