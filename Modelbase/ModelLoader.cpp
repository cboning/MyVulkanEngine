#include "AssimpGLMHelpers.h"
#include "ModelLoader.h"
#include "Animation.h"
#include "Mesh.h"
#include "Model.h"

namespace Modelbase
{
void ModelLoader::loadModel(Model &model, const std::string &fileName)
{
    Assimp::Importer importer;
    const aiScene *pScene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
        throw std::runtime_error("Failed to load model!");

    model._fileDirectory = fileName.substr(0, fileName.find_last_of("/"));

    processNode(model, pScene->mRootNode, pScene, model._rootNode);
    uint32_t animationCount = pScene->mNumAnimations;

    for (uint32_t i = 0; i < animationCount; ++i)
    {
        model._animations.emplace_back(pScene->mAnimations[i], model);
    }
}

void ModelLoader::processNode(Model &model, aiNode *pNode, const aiScene *pScene, ModelData::AssimpNodeData &src)
{
    for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
    {
        aiMesh *pMesh = pScene->mMeshes[pNode->mMeshes[i]];
        processMesh(model, pMesh, pScene);
    }

    src.name = pNode->mName.data;
    src.transformation = AssimpGLMHelpers::getGLMMat4(pNode->mTransformation);
    src.childrenCount = pNode->mNumChildren;

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
    {
        ModelData::AssimpNodeData nodeData;
        processNode(model, pNode->mChildren[i], pScene, nodeData);
        src.children.push_back(nodeData);
    }
}

void ModelLoader::initVertexBoneData(ModelData::Vertex &vertex)
{
    for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        vertex.boneIds[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}

void ModelLoader::setVertexBoneData(ModelData::Vertex &vertex, int boneId, float weight)
{
    for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.boneIds[i] < 0)
        {
            vertex.boneIds[i] = boneId;
            vertex.weights[i] = weight;
            break;
        }
    }
}


void ModelLoader::loadVerticesBoneWeight(Model &model, std::vector<ModelData::Vertex> &vertices, aiMesh *pMesh)
{
    for (uint32_t boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex)
    {
        int boneId;
        std::string boneName = pMesh->mBones[boneIndex]->mName.C_Str();
        if (!model._boneInfoMap.count(boneName))
        {
            ModelData::BoneInfo boneInfo;
            boneId = boneInfo.id = model._boneInfoMap.size();
            boneInfo.offset = AssimpGLMHelpers::getGLMMat4(pMesh->mBones[boneIndex]->mOffsetMatrix);
            model._boneInfoMap[boneName] = boneInfo;
        }
        else
            boneId = model._boneInfoMap[boneName].id;

        aiVertexWeight *weights = pMesh->mBones[boneIndex]->mWeights;
        int weightCount = pMesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < weightCount; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            setVertexBoneData(vertices[vertexId], boneId, weight);
        }
    }
}

void ModelLoader::processMesh(Model &model, aiMesh *pMesh, const aiScene *pScene)
{
    std::vector<ModelData::Vertex> vertices;
    std::vector<uint16_t> indices;
    for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
    {
        ModelData::Vertex vertex;
        initVertexBoneData(vertex);
        vertex.pos = AssimpGLMHelpers::getGLMVec3(pMesh->mVertices[i]);
        vertex.normal = AssimpGLMHelpers::getGLMVec3(pMesh->mNormals[i]);
        // vertex.color = AssimpGLMHelpers::getGLMVec4(pMesh->mColors[i]);
        if (pMesh->mTextureCoords[0])
        {
            vertex.texCoord.x = pMesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = pMesh->mTextureCoords[0][i].y;
        }
        else
            vertex.texCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    loadVerticesBoneWeight(model, vertices, pMesh);

    for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
    {
        aiFace face = pMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    const std::string meshName(pMesh->mName.C_Str());
    std::string pipelineName;
    if (model._meshPipelineNames.count(meshName))
        pipelineName = model._meshPipelineNames.at(meshName);
    else
        pipelineName = model._meshPipelineNames.at("default");

    const std::vector<aiTextureType> textureTypeFeatures = model._textureTypeFeatures.at(pipelineName);
    aiMaterial *pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
    std::vector<std::vector<std::string>> textureNames;
    textureNames.resize(textureTypeFeatures.size(), {"Empty"});

    for (uint32_t i = 0; i < textureTypeFeatures.size(); ++i)
        textureNames[i][0] = loadMaterialTextures(model, pMaterial, textureTypeFeatures[i])[0];
    model._meshes.emplace_back(meshName, model._deviceName, vertices, indices, textureNames, model._descriptorSets.name());
}


std::vector<std::string> ModelLoader::loadMaterialTextures(Model &model, aiMaterial *pMaterial, aiTextureType textureType)
{
    std::vector<std::string> textureNames;
    for (unsigned int i = 0; i < pMaterial->GetTextureCount(textureType); ++i)
    {
        aiString path;
        pMaterial->GetTexture(textureType, i, &path);
        std::string filename = model._fileDirectory + "/" + path.C_Str();
        if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, filename))
        {
            Vkbase::ResourceBase::resourceManager().create<Vkbase::Image>(filename, model._deviceName, filename, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D,
                              vk::ImageUsageFlagBits::eSampled);
            model._textureFiles.push_back(filename);
        }
        textureNames.push_back(filename);
    }
    if (textureNames.empty())
        textureNames.push_back("Empty");
    return textureNames;
}


}