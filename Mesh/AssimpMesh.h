#ifndef __ASSIMP_MESH__
#define __ASSIMP_MESH__

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"

class AssimpMesh : public Mesh {
 private:
  void processMesh(const aiMesh* mesh);
  void processNode(const aiScene* scene);
 public:
  AssimpMesh();
  AssimpMesh(std::string path);
  ~AssimpMesh();
  void import();
};

#endif
