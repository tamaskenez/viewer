#pragma once

#include "SceneToRender.h"

#include <filesystem>

SceneToRender import_to_scene_to_render(const std::filesystem::path& path, std::string glsl_version);
