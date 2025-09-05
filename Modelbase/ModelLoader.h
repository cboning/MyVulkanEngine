#pragma once
#include "ModelData.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>

namespace Modelbase
{
class Model;
class ModelLoader
{
    friend class Model;

  private:
    static void loadModel(Model &model, const std::string &filename);
    static void processNode(Model &model, aiNode *pNode, const aiScene *pScene, ModelData::AssimpNodeData &src);
    static void processMesh(Model &model, aiMesh *pMesh, const aiScene *pScene);
    static std::vector<std::string> loadMaterialTextures(Model &model, aiMaterial *pMaterial, aiTextureType textureType);
    static void initVertexBoneData(ModelData::Vertex &vertex);
    static void setVertexBoneData(ModelData::Vertex &vertex, int boneId, float weight);
    static void loadVerticesBoneWeight(Model &model, std::vector<ModelData::Vertex> &vertices, aiMesh *pMesh);
};
} // namespace Modelbase
