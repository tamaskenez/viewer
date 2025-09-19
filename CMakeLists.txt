cmake_minimum_required(VERSION 3.31)
project(viewer)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (EMSCRIPTEN)
	if(NOT CMAKE_BUILD_TYPE)
		message(FATAL_ERROR "CMAKE_BUILD_TYPE must be set when building for emscripten.")
	endif()

	macro(append_linker_flags FLAGS)
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
	endmacro()

	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# append_linker_flags("-sASYNCIFY")

	# Enable Closure compiler for aggressive JS size minification
	#append_linker_flags("--closure 1")

	# Enable aggressive MINIMAL_RUNTIME mode.
	# append_linker_flags("-sMINIMAL_RUNTIME=2")

	# Require WebGL 2 support in target browser, for smallest generated code size. (pass -sMIN_WEBGL_VERSION=1 to dual-target WebGL 1 and WebGL 2)
	append_linker_flags("-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")

	# The generated build output is only to be expected to be run in a web browser, never in a native CLI shell, or in a web worker.
	append_linker_flags("-sENVIRONMENT=web")

	# Choose the oldest browser versions that should be supported. The higher minimum bar you choose, the less
	# emulation code may be present for old browser quirks.
	append_linker_flags("-sMIN_FIREFOX_VERSION=100")
	append_linker_flags("-sMIN_SAFARI_VERSION=160000")
	append_linker_flags("-sMIN_CHROME_VERSION=100")

	if(CMAKE_BUILD_TYPE STREQUAL Debug)
		append_linker_flags("-O0")
	elseif(CMAKE_BUILD_TYPE STREQUAL Release)
		# Optimization flag to optimize aggressively for size. (other options -Os, -O3, -O2, -O1, -O0)
		append_linker_flags("-Oz")
	else()
		message(FATAL_ERROR "Unhandled CMAKE_BUILD_TYPE value: ${CMAKE_BUILD_TYPE}")
	endif()

else()
	find_package(OpenGL REQUIRED)
endif()

set(BUILD_SHARED_LIBS 0)

# ASSIMP
set(ASSIMP_BUILD_TESTS 0)
set(ASSIMP_INSTALL 0)
set(ASSIMP_IGNORE_GIT_HASH 1)
set(ASSIMP_NO_EXPORT)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT 0)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT 0)
foreach(format OBJ STL GLTF COLLADA)
	set(ASSIMP_BUILD_${format}_IMPORTER 1)
endforeach()
if(EMSCRIPTEN)
	set(ASSIMP_BUILD_ZLIB 1)
else()
	set(ASSIMP_BUILD_ZLIB 0)
endif()
add_subdirectory(_submodules/assimp)

# SDL

add_subdirectory(_submodules/SDL)

# IMGUI

set(imgui_root ${PROJECT_SOURCE_DIR}/_submodules/imgui)
set(sources
    ${imgui_root}/imgui.cpp
    ${imgui_root}/imgui.h
    ${imgui_root}/imgui_demo.cpp
    ${imgui_root}/imgui_draw.cpp
    ${imgui_root}/imgui_internal.h
    ${imgui_root}/imgui_tables.cpp
    ${imgui_root}/imgui_widgets.cpp
    ${imgui_root}/imstb_rectpack.h
    ${imgui_root}/imstb_textedit.h
    ${imgui_root}/imstb_truetype.h
)
add_library(imgui STATIC ${sources})
source_group(TREE ${imgui_root} FILES ${sources})
target_include_directories(imgui PUBLIC $<BUILD_INTERFACE:${imgui_root}>)
add_library(imgui::imgui ALIAS imgui)

# IMGUI BACKEND: SDL3 + OPENGL3

set(sources
    ${imgui_root}/backends/imgui_impl_opengl3.cpp
    ${imgui_root}/backends/imgui_impl_opengl3.h
    ${imgui_root}/backends/imgui_impl_opengl3_loader.h
    ${imgui_root}/backends/imgui_impl_sdl3.cpp
    ${imgui_root}/backends/imgui_impl_sdl3.h
)
source_group(TREE ${imgui_root}/backends FILES ${sources})
add_library(imgui_backend_sdl3_opengl3 STATIC ${sources})
target_link_libraries(imgui_backend_sdl3_opengl3
    PUBLIC
        SDL3::SDL3
        imgui::imgui
)
if(NOT EMSCRIPTEN)
	target_link_libraries(imgui_backend_sdl3_opengl3 PUBLIC OpenGL::GL)
endif()

target_include_directories(imgui_backend_sdl3_opengl3 PUBLIC ${imgui_root}/backends)

# ----

include(_submodules/meadow/cmake/warnings_clang.cmake)
include(_submodules/meadow/cmake/warnings_gcc.cmake)
include(_submodules/meadow/cmake/warnings_msvc.cmake)

set(MEADOW_NO_EIGEN 1)
set(MEADOW_NO_ABSL 1)

add_subdirectory(_submodules/meadow)
add_subdirectory(src)
