#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <json.hpp>
#include <unordered_map>
#include <unordered_set>

#include "../Vkbase/Vkbase.h"

#include "../Object/Object.h"
#include "ModelData.h"

#define MAX_BONES 500
using json = nlohmann::json;

namespace Modelbase
{
template <typename T> class Mesh;
class Animation;
class ModelInstance;
struct AnimationIndex;

struct ModelUniformData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 bonesMatrices[MAX_BONES];
};

class Model
{
    friend class ModelLoader;
  private:
    const std::string _deviceName;

    std::string _fileDirectory;

    std::vector<Mesh<ModelData::Vertex>> _meshes;
    std::vector<std::string> _textureFiles;

    ModelData::AssimpNodeData _rootNode; // 更新动画要用

    std::unordered_map<std::string, ModelData::BoneInfo> _boneInfoMap; // 与顶点输入中的权重与骨骼ID匹配

    int _boneCount = 0;

    std::vector<Animation> _animations;

    const vk::Sampler &_sampler;

    std::vector<ModelInstance *> _pInstances;

    const std::unordered_map<std::string, std::vector<aiTextureType>> _textureTypeFeatures;

    const std::unordered_map<std::string, std::string> _meshPipelineNames;

    Vkbase::DescriptorSets &_descriptorSets; // 用于描述纹理与采样器，也提供UBO的layout，但不描述UBO

    std::unordered_map<std::string, uint32_t> _instanceIndexMap;

    void createDescriptorSets(Vkbase::DescriptorSets &descriptorSets) const;
    void addUBODescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) const;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) const;
    void applyTextureDescriptorSetConfig();
    void writeTextureDescriptorSets(const vk::Sampler &sampler) const;
    static std::unordered_map<std::string, std::vector<aiTextureType>> getTextureFeaturesWithConfig(const json &config);
    static std::vector<aiTextureType> getTextureTypeWithConfig(const json &config);
    static aiTextureType getTextureTypeWithString(const std::string &textureType);
    static std::unordered_map<std::string, std::string> getMeshToPipelineNamesWithConfig(const json &config);

    inline static std::unordered_set<Model *> _models;

  public:
    Model(const std::string &deviceName, const vk::Sampler &sampler, const std::string &fileName,
          const std::unordered_map<std::string, std::vector<aiTextureType>> &textureTypeFeatures,
          const std::unordered_map<std::string, std::string> &meshPipelineNames);
    Model(const std::string &deviceName, const vk::Sampler &sampler, json config);
    ~Model();
    void draw(uint32_t currentFrame, const vk::CommandBuffer &commandBuffer, uint32_t instanceIndex);
    std::unordered_map<std::string, ModelData::BoneInfo> &boneInfoMap();
    int &boneCount();
    ModelData::AssimpNodeData *rootNode();
    void updateAnimation(uint32_t instanceIndex, float deltaTick);
    void updateAnimation(const std::string &instanceName, float deltaTick);
    void updateAnimation(float deltaTick);
    std::vector<vk::DescriptorSetLayout> descriptorSetLayout(uint32_t instanceIndex, const std::string &pipelineName) const;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayout(const std::string &instanceName, const std::string &pipelineName) const;
    uint32_t animationCount() const;
    Animation &animation(uint32_t index);
    ModelInstance &createNewInstance(const std::string &instanceName, const AnimationIndex &basedAnimationIndex);
    int32_t instanceIndex(const std::string &instanceName) const;
    void removeInstance(const std::string &instanceName);
    ModelInstance &instance(uint32_t instanceIndex);
    ModelInstance &instance(const std::string &instanceName);

    static const std::unordered_set<Model *> &models();
};
}; // namespace Modelbase
