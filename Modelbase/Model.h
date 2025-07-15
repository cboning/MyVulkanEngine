#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <unordered_set>
#include "../Vkbase/Vkbase.h"

#include "../Camera.h"

#include "ModelData.h"

#define MAX_BONES 300

namespace Modelbase
{
    class Mesh;
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

    struct AnimationInstance
    {
        float animationProgress;
        bool isAnimationIndexStackLock = false;
        Vkbase::DescriptorSets &descriptorSets;
        std::vector<AnimationIndex> animationIndexStack;

        AnimationInstance(const std::string &descriptorSetsName, const std::string &deviceName)
            : descriptorSets(*(new Vkbase::DescriptorSets(descriptorSetsName, deviceName)))
        {
        };
        ~AnimationInstance()
        {
            descriptorSets.destroy();
        }
    };

    class Model
    {
    private:
    
        const Vkbase::Device &_device;
        const Vkbase::CommandPool &_commandPool;

        std::string _fileDirectory;
        Assimp::Importer _importer;

        std::vector<Mesh> _meshs;
        std::vector<std::string> _files;
        std::vector<std::array<unsigned int, 4>> _imageDescriptorSetMasks;

        ModelData::AssimpNodeData _rootNode; //更新动画要用

        std::unordered_map<std::string, ModelData::BoneInfo> _boneInfoMap; //与顶点输入中的权重与骨骼ID匹配

        int _boneCount = 0;
        int _animationIndex = 0;

        std::vector<Animation *> _pAnimations;

        vk::Sampler _sampler;

        uint32_t _animationCount;

        std::vector<uint32_t> _removedInstanceIndices; // 无序

        std::unordered_map<std::string, uint32_t> _instanceIndexMap;

        std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> _descriptorTypes;

        std::vector<AnimationInstance> _animationInstances;

        const std::vector<aiTextureType> _textureTypeFeatures;

        std::unordered_map<std::string, std::string> _meshPipelineNames;

        Vkbase::DescriptorSets &_descriptorSets;

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

        inline static std::unordered_set<Model *> _models;
        
    public:
        Model(const std::string &deviceName, const std::string &commandPoolName, const std::string &fileName, const std::vector<aiTextureType> &textureTypeFeatures, vk::Sampler sampler);
        ~Model();
        void draw(uint32_t currentFrame, const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, uint32_t instanceIndex);
        std::unordered_map<std::string, ModelData::BoneInfo> &boneInfoMap();
        int &boneCount();
        ModelData::AssimpNodeData *rootNode();
        void updateAnimation(uint32_t instanceIndex, float deltaTick);
        void updateAnimation(const std::string &instanceName, float deltaTick);
        std::vector<vk::DescriptorSetLayout> descriptorSetLayout(uint32_t instanceIndex) const;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayout(const std::string &instanceName) const;
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
        uint32_t instanceIndex(const std::string &instanceName) const;
        void removeInstance(const std::string &instanceName);

        static const std::unordered_set<Model *> &models();
        
    };
};
