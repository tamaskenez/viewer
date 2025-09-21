#include "import3d.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meadow/cppext.h>

void import_to_scene_to_render(const std::filesystem::path& path)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality | aiProcess_PreTransformVertices);
    NOP;
}
