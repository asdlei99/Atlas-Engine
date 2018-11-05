#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "../System.h"
#include "../Mesh/MeshData.h"

#include "../libraries/assimp/Importer.hpp"
#include "../libraries/assimp/scene.h"
#include "../libraries/assimp/postprocess.h"
#include "../libraries/assimp/types.h"
#include "../libraries/assimp/config.h"

class ModelLoader {

public:
	static MeshData* LoadMesh(const char* filename);

private:
	static Material* LoadMaterial(aiMaterial* assimpMaterial, string directory);

};


#endif