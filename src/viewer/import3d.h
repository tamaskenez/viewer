#pragma once

#include "SceneToRender.h"

#include <expected>
#include <filesystem>

SceneToRender import_to_scene_to_render_from_file(const std::filesystem::path& path, std::string glsl_version);
std::expected<SceneToRender, std::string>
import_to_scene_to_render_from_text(std::string_view text, std::string glsl_version);
