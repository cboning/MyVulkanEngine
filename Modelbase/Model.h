#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <json.hpp>
#include <unordered_map>
#include <unordered_set>

#include "../Vkbase/Vkbase.h"

#include "../Camera.h"

#include "../Object/Object.h"
#include "ModelData.h"

#define MAX_BONES 500
using json = nlohmann::json;

namespace Modelbase
{
template <typename T> class Mesh;
class Animation;

struct ModelUniformData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 bonesMatrices[MAX_BONES];
};

struct AnimationIndex
{
    uint32_t animationIndex;
    float beginTime = 0.0f;
};

struct Instance
{
    float animationProgress;
    bool isAnimationIndexStackLock = false;
    Vkbase::DescriptorSets &descriptorSets;
    std::vector<AnimationIndex> animationIndexStack;
    Object object;

    Instance(const std::string &descriptorSetsName, const std::string &deviceName)
        : descriptorSets(*(new Vkbase::DescriptorSets(descriptorSetsName, deviceName))) {};
    ~Instance() { descriptorSets.destroy(); }
};

class Model
{
  private:
    const Vkbase::Device &_device;
    const Vkbase::CommandPool &_commandPool;

    std::string _fileDirectory;
    Assimp::Importer _importer;

    std::vector<Mesh<ModelData::Vertex>> _meshs;
    std::vector<std::string> _textureFiles;

    ModelData::AssimpNodeData _rootNode; // 更新动画要用

    std::unordered_map<std::string, ModelData::BoneInfo> _boneInfoMap; // 与顶点输入中的权重与骨骼ID匹配

    int _boneCount = 0;

    std::vector<Animation *> _pAnimations;
    uint32_t _animationCount;

    vk::Sampler _sampler;

    std::vector<uint32_t> _removedInstanceIndices; // 无序

    std::vector<Instance> _instances;

    const std::unordered_map<std::string, std::vector<aiTextureType>> _textureTypeFeatures;

    const std::unordered_map<std::string, std::string> _meshPipelineNames;

    Vkbase::DescriptorSets &_descriptorSets;

    std::unordered_map<std::string, uint32_t> _instanceIndexMap;

    void loadModel(const std::string &fileName);
    void processNode(aiNode *pNode, const aiScene *pScene, ModelData::AssimpNodeData &src);
    void processMesh(aiMesh *pMesh, const aiScene *pScene);
    std::vector<std::string> loadMaterialTextures(aiMaterial *pMaterial, aiTextureType textureType);
    void initVertexBoneData(ModelData::Vertex &vertex);
    void setVertexBoneData(ModelData::Vertex &vertex, int boneId, float weight);
    void loadVerticesBoneWeight(std::vector<ModelData::Vertex> &vertices, aiMesh *pMesh);
    void createDescriptorSets(uint32_t instanceIndex);
    void addUBODescriptorSetsConfig(uint32_t instanceIndex);
    void writeDescriptorSets(uint32_t instanceIndex);
    void applyTextureDescriptorSetConfig();
    void writeTextureDescriptorSets(const vk::Sampler &sampler) const;
    static std::unordered_map<std::string, std::vector<aiTextureType>> getTextureFeaturesWithConfig(const json &config);
    static std::vector<aiTextureType> getTextureTypeWithConfig(const json &config);
    static aiTextureType getTextureTypeWithString(const std::string &textureType);
    static std::unordered_map<std::string, std::string> getMeshToPipelineNamesWithConfig(const json &config);

    inline static std::unordered_set<Model *> _models;

  public:
    Model(const std::string &deviceName, const std::string &commandPoolName, vk::Sampler sampler, const std::string &fileName,
          const std::unordered_map<std::string, std::vector<aiTextureType>> &textureTypeFeatures,
          const std::unordered_map<std::string, std::string> &meshPipelineNames);
    Model(const std::string &deviceName, const std::string &commandPoolName, vk::Sampler sampler, json config);
    ~Model();
    void draw(uint32_t currentFrame, const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, uint32_t instanceIndex);
    std::unordered_map<std::string, ModelData::BoneInfo> &boneInfoMap();
    int &boneCount();
    ModelData::AssimpNodeData *rootNode();
    void updateAnimation(uint32_t instanceIndex, float deltaTick);
    void updateAnimation(const std::string &instanceName, float deltaTick);
    void updateAnimation(float deltaTick);
    std::vector<vk::DescriptorSetLayout> descriptorSetLayout(uint32_t instanceIndex, const std::string &pipelineName) const;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayout(const std::string &instanceName, const std::string &pipelineName) const;
    uint32_t animationCount();
    void addAnimationIndexToStack(uint32_t instanceIndex, std::vector<AnimationIndex> animationIndices);
    void addAnimationIndexToStack(const std::string &instanceName, std::vector<AnimationIndex> animationIndices);
    void updateUniformBuffers(uint32_t instanceIndex, uint32_t currentFrame, const Camera &camera);
    void updateUniformBuffers(const std::string &instanceName, uint32_t currentFrame, const Camera &camera);
    void setBasicAnimation(uint32_t instanceIndex, uint32_t animationIndex);
    void setBasicAnimation(const std::string &instanceName, uint32_t animationIndex);
    void createNewInstance(const std::string &instanceName, AnimationIndex basedAnimationIndex);
    bool canAddAnimation(uint32_t instanceIndex);
    bool canAddAnimation(const std::string &instanceName);
    int32_t instanceIndex(const std::string &instanceName) const;
    void removeInstance(const std::string &instanceName);
    Object &instanceObject(uint32_t instanceIndex);
    Object &instanceObject(const std::string &instanceName);

    static const std::unordered_set<Model *> &models();
};
}; // namespace Modelbase
