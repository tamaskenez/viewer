#include "import3d.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meadow/cppext.h>

void f()
{
    Assimp::Importer importer;
    auto scene =
      importer.ReadFile("/Users/dk8TamKe/Downloads/Untitled Model.zip", aiProcessPreset_TargetRealtime_Quality);
    NOP;
}
