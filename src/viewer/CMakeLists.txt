file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.h *.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(viewer MACOSX_BUNDLE WIN32
	${sources}
)
