#include "Model.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "ModelInstance.h"
#include "ModelLoader.h"
#include "Modelbase.h"
#include <iostream>

namespace Modelbase
{

Model::Model(const std::string &deviceName, const vk::Sampler &sampler, const std::string &fileName,
             const std::unordered_map<std::string, std::vector<aiTextureType>> &textureTypeFeatures,
             const std::unordered_map<std::string, std::string> &meshPipelineNames)
    : _deviceName(deviceName), _sampler(sampler), _textureTypeFeatures(textureTypeFeatures), _meshPipelineNames(meshPipelineNames),
      _descriptorSets(createResource<Vkbase::DescriptorSets>("", deviceName))
{
    if (auto p = _descriptorSets.lock())
        p->setLock();
    _models.insert(this);
    if (!Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Image, "Empty").lock())
        createResource<Vkbase::Image>("Empty", deviceName, 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D,
                                      vk::ImageUsageFlagBits::eSampled, (uint32_t[]){0xFFFF00FF});

    _textureFiles.push_back("Empty");

    ModelLoader::loadModel(*this, fileName);

    if (auto p = _descriptorSets.lock<Vkbase::DescriptorSets>())
    {
        p->addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, 0);
        applyTextureDescriptorSetConfig();
        p->init();
    }

    writeTextureDescriptorSets(_sampler);
}

Model::Model(const std::string &deviceName, const vk::Sampler &sampler, json config)
    : Model(deviceName, sampler, config["filename"], getTextureFeaturesWithConfig(config), getMeshToPipelineNamesWithConfig(config))
{
}

Model::~Model()
{
    for (const ModelInstance *pInstance : _pInstances)
        delete pInstance;
    _models.erase(this);
}

void Model::createDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets)
{
    addUBODescriptorSetsConfig(descriptorSets);
    if (auto p = descriptorSets.lock<Vkbase::DescriptorSets>())
        p->init();
    writeDescriptorSets(descriptorSets);
}

void Model::draw(uint32_t currentFrame, const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &,
                 const std::string &pipelineName, uint32_t instanceIndex) const
{
    std::vector<std::pair<Vkbase::VkResourceManagerHolder::WeakReference, std::pair<std::string, uint32_t>>> descriptorSets;
    descriptorSets.reserve(_textureTypeFeatures.at(pipelineName).size() + 1);
    descriptorSets.push_back({_pInstances[instanceIndex]->descriptorSets(), {"UBO", currentFrame}});

    for (const std::unique_ptr<Vkbase::Mesh<ModelData::Vertex>> &mesh : _pMeshes)
    {
        const std::string &meshPipeline = _meshPipelineNames.count(mesh->name()) ? _meshPipelineNames.at(mesh->name()) : _meshPipelineNames.at("default");

        if (pipelineName != meshPipeline)
            continue;

        // Reset descriptorSets to initial size of 1 (keeping UBO)
        descriptorSets.resize(1);

        // Add texture descriptors
        for (const auto &textureName : mesh->textureNames())
        {
            descriptorSets.push_back({_descriptorSets, {textureName[0], 0}});
        }

        mesh->draw(commandBuffer, descriptorSets);
    }
}

void Model::updateAnimation(uint32_t instanceIndex, float deltaTick)
{
    if (_animations.size() < 1)
        return;

    if (instanceIndex >= _pInstances.size())
    {
        std::cout << "[Error Model] Can't find the object." << std::endl;
        return;
    }
    _pInstances[instanceIndex]->updateAnimation(deltaTick);
}

void Model::updateAnimation(const std::string &instanceName, float deltaTick) { updateAnimation(instanceIndex(instanceName), deltaTick); }

void Model::updateAnimation(float deltaTick)
{
    for (auto instanceIndex : _instanceIndexMap)
        updateAnimation(instanceIndex.second, deltaTick);
}

void Model::applyTextureDescriptorSetConfig()
{
    uint32_t count = 0;
    for (const std::string &textureFile : _textureFiles)
    {
        if (auto p = _descriptorSets.lock<Vkbase::DescriptorSets>())
        {
            p->addDescriptorSetCreateConfig(textureFile, {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, 1,
                                            count ? std::pair<Vkbase::VkResourceManagerHolder::WeakReference, std::string>{_descriptorSets, _textureFiles[0]}
                                                  : std::pair<Vkbase::VkResourceManagerHolder::WeakReference, std::string>{{}, ""});
            ++count;
        }
    }
}

void Model::addUBODescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets) const
{
    if (auto p = descriptorSets.lock<Vkbase::DescriptorSets>())
        p->addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, MAX_FLIGHT_COUNT,
                                        std::pair<Vkbase::VkResourceManagerHolder::WeakReference, std::string>{_descriptorSets, "UBO"});
}

void Model::writeTextureDescriptorSets(const vk::Sampler &sampler) const
{
    std::vector<std::pair<vk::DescriptorImageInfo, Vkbase::VkResourceManagerHolder::WeakReference>> imageInfo(1);
    imageInfo[0].first.setSampler(sampler);
    if (auto p = _descriptorSets.lock<Vkbase::DescriptorSets>())
        for (const std::string &file : _textureFiles)
        {
            imageInfo[0].first.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            imageInfo[0].second = Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Image, file);
            p->writeSets(file, 0, {}, imageInfo, 1);
        }
}

void Model::writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets)
{
    if (auto p = descriptorSets.lock<Vkbase::DescriptorSets>())
    {
        std::vector<std::pair<vk::DescriptorBufferInfo, Vkbase::VkResourceManagerHolder::WeakReference>> bufferInfos(
            MAX_FLIGHT_COUNT, {vk::DescriptorBufferInfo().setOffset(0).setRange(sizeof(ModelUniformData)), {}});
        uint32_t count = 0;
        for (auto &bufferInfo : bufferInfos)
        {
            bufferInfo.second = createResource<Vkbase::Buffer>(p->name() + "_UBO_" + std::to_string(count), _deviceName, sizeof(ModelUniformData),
                                                               vk::BufferUsageFlagBits::eUniformBuffer);

            ++count;
        }

        p->writeSets("UBO", 0, bufferInfos, {}, MAX_FLIGHT_COUNT);
    }
}

std::unordered_map<std::string, std::vector<aiTextureType>> Model::getTextureFeaturesWithConfig(const json &config)
{
    if (!config.count("pipelines"))
        throw std::runtime_error("Model config doesn't have pipelines features.");

    const json &pipelines = config["pipelines"];
    std::unordered_map<std::string, std::vector<aiTextureType>> textureFeatures;
    for (const json &pipeline : pipelines)
        textureFeatures[pipeline["name"]] = getTextureTypeWithConfig(pipeline["textureFeatures"]);

    return textureFeatures;
}

std::vector<aiTextureType> Model::getTextureTypeWithConfig(const json &config)
{
    std::vector<aiTextureType> textureTypes;

    textureTypes.reserve(config.size());
    for (json textureTypeString : config)
        textureTypes.push_back(JsonConfigReader::getTextureTypeWithString(textureTypeString));
    return textureTypes;
}

std::unordered_map<std::string, std::string> Model::getMeshToPipelineNamesWithConfig(const json &config)
{
    if (!config.count("meshes"))
        throw std::runtime_error("Model config doesn't have default");

    const json &meshPipelineNamesConfig = config["meshes"];
    std::unordered_map<std::string, std::string> meshPipelineNames;

    for (const json &meshPipelineName : meshPipelineNamesConfig)
        meshPipelineNames[meshPipelineName["name"]] = meshPipelineName["pipeline"];

    if (!meshPipelineNames.count("default"))
        throw std::runtime_error("Model config doesn't have default");

    return meshPipelineNames;
}

ModelInstance &Model::createNewInstance(const std::string &instanceName, const AnimationIndex &basedAnimationIndex)
{
    if (_instanceIndexMap.find(instanceName) != _instanceIndexMap.end())
    {
        std::cout << "[Error Model] There is a instance was created, it's name is " << instanceName << ", so you can't create a new instance have the same name"
                  << std::endl;
        return instance(instanceName);
    }

    _instanceIndexMap[instanceName] = _pInstances.size();
    ModelInstance *pAnimationInstance = new ModelInstance(_deviceName, *this);
    _pInstances.push_back(pAnimationInstance);

    createDescriptorSets(pAnimationInstance->descriptorSets());

    pAnimationInstance->setBasicAnimation(basedAnimationIndex);
    return *pAnimationInstance;
}

std::unordered_map<std::string, ModelData::BoneInfo> &Model::boneInfoMap() { return _boneInfoMap; }

int &Model::boneCount() { return _boneCount; }

ModelData::AssimpNodeData *Model::rootNode() { return &_rootNode; }

std::vector<vk::DescriptorSetLayout> Model::descriptorSetLayout(uint32_t instanceIndex, const std::string &pipelineName) const
{
    std::vector<vk::DescriptorSetLayout> layouts;
    if (auto p = _pInstances[instanceIndex]->descriptorSets().lock<Vkbase::DescriptorSets>())
        layouts.push_back(p->layout("UBO"));
    for (uint32_t i = 0; i < _textureTypeFeatures.at(pipelineName).size(); ++i)
        if (auto p = _descriptorSets.lock<Vkbase::DescriptorSets>())
            layouts.push_back(p->layout(_textureFiles[0]));
    return layouts;
}

std::vector<vk::DescriptorSetLayout> Model::descriptorSetLayout(const std::string &instanceName, const std::string &pipelineName) const
{
    return descriptorSetLayout(instanceIndex(instanceName), pipelineName);
}

Animation &Model::animation(uint32_t index) { return _animations[index]; }

uint32_t Model::animationCount() const { return _animations.size(); }

int32_t Model::instanceIndex(const std::string &instanceName) const
{
    if (_instanceIndexMap.find(instanceName) == _instanceIndexMap.end())
    {
        std::cout << "[Model Error] There is not instance name is " << instanceName << std::endl;
        return -1;
    }

    return _instanceIndexMap.at(instanceName);
}

ModelInstance &Model::instance(uint32_t instanceIndex) { return *_pInstances[instanceIndex]; }

const ModelInstance &Model::instance(uint32_t instanceIndex) const { return *_pInstances[instanceIndex]; }

ModelInstance &Model::instance(const std::string &instanceName) { return instance(instanceIndex(instanceName)); }

const ModelInstance &Model::instance(const std::string &instanceName) const { return instance(instanceIndex(instanceName)); }

void Model::removeInstance(const std::string &instanceName) { _instanceIndexMap.erase(instanceName); }

const std::unordered_set<Model *> &Model::models() { return _models; }

const std::vector<std::unique_ptr<Vkbase::Mesh<ModelData::Vertex>>> &Model::meshes() const { return _pMeshes; }

} // namespace Modelbase