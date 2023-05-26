#include "AssimpMesh.h"
#include <iostream>

AssimpMesh::AssimpMesh(std::string path):Mesh(path){};
AssimpMesh::~AssimpMesh(){};

// Assimp import mesh
void AssimpMesh::processMesh(const aiMesh* mesh) {
  // Vertex Positions
  for (int i = 0; i < mesh->mNumVertices; i++) {
    aiVector3D pos = mesh->mVertices[i];
    vertex_data.push_back(pos.x);
    vertex_data.push_back(pos.y);
    vertex_data.push_back(pos.z);
  }
  // Normals
  if (!mesh->HasNormals())
    return;
  for (int i = 0; i < mesh->mNumVertices; i++) {
    aiVector3D normal = mesh->mNormals[i];
    vertex_normal.push_back(normal.x);
    vertex_normal.push_back(normal.y);
    vertex_normal.push_back(normal.z);
  }
  // ToDo: Get Colors
  unsigned int mColorChannel = mesh->GetNumColorChannels();
  std::cout << mColorChannel << std::endl;
  // Index
  if (!mesh->HasFaces())
    return;
  int mFaceCount = mesh->mNumFaces;
  for (int i = 0; i < mFaceCount; i++) {
    aiFace& face = mesh->mFaces[i];
    if (face.mNumIndices == 3) {
      index_data.push_back(face.mIndices[0]);
      index_data.push_back(face.mIndices[1]);
      index_data.push_back(face.mIndices[2]);
    }
  }
}

void AssimpMesh::processNode(const aiScene *scene) {
  for (int i = 0; i < scene->mNumMeshes; i++)
    processMesh(scene->mMeshes[i]);
}

void AssimpMesh::import() {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(file_path,
					   aiProcess_CalcTangentSpace       |
					   aiProcess_Triangulate            |
					   aiProcess_JoinIdenticalVertices  |
					   aiProcess_SortByPType);
  if (!scene || !scene->mRootNode) {
    std::cout << "Failed to read file: " << importer.GetErrorString() << std::endl;
    return;
  }
  processNode(scene);
}
