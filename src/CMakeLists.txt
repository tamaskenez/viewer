
if(NOT EMSCRIPTEN)
	file(GLOB_RECURSE sources CONFIGURE_DEPENDS glad/*.h glad/*.c)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/glad FILES ${sources})
	add_library(glad-opengl_4_1 STATIC ${sources})
	target_include_directories(glad-opengl_4_1 PUBLIC glad/include)
endif()

add_subdirectory(viewer)
