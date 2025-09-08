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
      _descriptorSets(*(Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>(fileName, deviceName)))
{
    _models.insert(this);
    if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, "Empty"))
        Vkbase::ResourceBase::resourceManager().create<Vkbase::Image>("Empty", deviceName, 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D,
                                                                      vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled, (uint32_t[]){0xFFFF00FF});

    _textureFiles.push_back("Empty");

    ModelLoader::loadModel(*this, fileName);

    _descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, 0);
    applyTextureDescriptorSetConfig();
    _descriptorSets.init();

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

    _descriptorSets.destroy();
}

void Model::createDescriptorSets(Vkbase::DescriptorSets &descriptorSets) const
{
    addUBODescriptorSetsConfig(descriptorSets);
    descriptorSets.init();
    writeDescriptorSets(descriptorSets);
}

void Model::draw(uint32_t currentFrame, const vk::CommandBuffer &commandBuffer, uint32_t instanceIndex)
{
    for (Mesh<ModelData::Vertex> &mesh : _meshes)
    {
        std::vector<vk::DescriptorSet> descriptorSets;
        descriptorSets.push_back(_pInstances[instanceIndex]->descriptorSets().sets("UBO")[currentFrame]);
        const std::vector<std::vector<std::string>> &textureNames = mesh.textureNames();

        for (const std::vector<std::string> &textureName : textureNames)
            descriptorSets.push_back(_descriptorSets.sets(textureName[0])[0]);

        const Vkbase::Pipeline &pipeline = *dynamic_cast<Vkbase::Pipeline *>(Vkbase::ResourceBase::resourceManager().resource(
            Vkbase::ResourceType::Pipeline, _meshPipelineNames.count(mesh.name()) ? _meshPipelineNames.at(mesh.name()) : _meshPipelineNames.at("default")));
        mesh.draw(commandBuffer, pipeline, descriptorSets);
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
        _descriptorSets.addDescriptorSetCreateConfig(textureFile, {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, 1,
                                                     count ? std::pair<const Vkbase::DescriptorSets *, std::string>{&_descriptorSets, _textureFiles[0]}
                                                           : std::pair<const Vkbase::DescriptorSets *, std::string>{nullptr, ""});
        ++count;
    }
}

void Model::addUBODescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) const
{
    descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, MAX_FLIGHT_COUNT,
                                                std::pair<const Vkbase::DescriptorSets *, std::string>{&_descriptorSets, "UBO"});
}

void Model::writeTextureDescriptorSets(const vk::Sampler &sampler) const
{
    std::vector<vk::DescriptorImageInfo> imageInfo(1);
    imageInfo[0].setSampler(sampler);
    for (const std::string &file : _textureFiles)
    {
        const Vkbase::Image &image = *dynamic_cast<const Vkbase::Image *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, file));
        imageInfo[0].setImageView(image.view()).setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        _descriptorSets.writeSets(file, 0, {}, imageInfo, 1);
    }
}

void Model::writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) const
{
    std::vector<vk::DescriptorBufferInfo> bufferInfos(MAX_FLIGHT_COUNT, vk::DescriptorBufferInfo().setOffset(0).setRange(sizeof(ModelUniformData)));
    uint32_t count = 0;
    for (vk::DescriptorBufferInfo &bufferInfo : bufferInfos)
    {
        bufferInfo.setBuffer(
            (Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>(descriptorSets.name() + "_UBO_" + std::to_string(count), _deviceName,
                                                                            sizeof(ModelUniformData), vk::BufferUsageFlagBits::eUniformBuffer))
                ->buffer());
        ++count;
    }

    descriptorSets.writeSets("UBO", 0, bufferInfos, {}, MAX_FLIGHT_COUNT);
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
        textureTypes.push_back(getTextureTypeWithString(textureTypeString));
    return textureTypes;
}

aiTextureType Model::getTextureTypeWithString(const std::string &textureType)
{
    static const std::unordered_map<std::string, aiTextureType> typeMap = {{"NONE", aiTextureType_NONE},
                                                                           {"DIFFUSE", aiTextureType_DIFFUSE},
                                                                           {"SPECULAR", aiTextureType_SPECULAR},
                                                                           {"AMBIENT", aiTextureType_AMBIENT},
                                                                           {"EMISSIVE", aiTextureType_EMISSIVE},
                                                                           {"HEIGHT", aiTextureType_HEIGHT},
                                                                           {"NORMALS", aiTextureType_NORMALS},
                                                                           {"SHININESS", aiTextureType_SHININESS},
                                                                           {"OPACITY", aiTextureType_OPACITY},
                                                                           {"DISPLACEMENT", aiTextureType_DISPLACEMENT},
                                                                           {"LIGHTMAP", aiTextureType_LIGHTMAP},
                                                                           {"REFLECTION", aiTextureType_REFLECTION},
                                                                           {"BASE_COLOR", aiTextureType_BASE_COLOR},
                                                                           {"NORMAL_CAMERA", aiTextureType_NORMAL_CAMERA},
                                                                           {"EMISSION_COLOR", aiTextureType_EMISSION_COLOR},
                                                                           {"METALNESS", aiTextureType_METALNESS},
                                                                           {"DIFFUSE_ROUGHNESS", aiTextureType_DIFFUSE_ROUGHNESS},
                                                                           {"AMBIENT_OCCLUSION", aiTextureType_AMBIENT_OCCLUSION},
                                                                           {"SHEEN", aiTextureType_SHEEN},
                                                                           {"CLEARCOAT", aiTextureType_CLEARCOAT},
                                                                           {"TRANSMISSION", aiTextureType_TRANSMISSION},
                                                                           {"MAYA_BASE", aiTextureType_MAYA_BASE},
                                                                           {"MAYA_SPECULAR", aiTextureType_MAYA_SPECULAR},
                                                                           {"MAYA_SPECULAR_COLOR", aiTextureType_MAYA_SPECULAR_COLOR},
                                                                           {"MAYA_SPECULAR_ROUGHNESS", aiTextureType_MAYA_SPECULAR_ROUGHNESS},
                                                                           {"ANISOTROPY", aiTextureType_ANISOTROPY},
                                                                           {"GLTF_METALLIC_ROUGHNESS", aiTextureType_GLTF_METALLIC_ROUGHNESS},
                                                                           {"UNKNOWN", aiTextureType_UNKNOWN}};

    auto it = typeMap.find(textureType);
    if (it != typeMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown texture type: " + textureType);
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
    ModelInstance *pAnimationInstance = new ModelInstance("Model_" + instanceName, _deviceName, *this);
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
    layouts.push_back(_pInstances[instanceIndex]->descriptorSets().layout("UBO"));
    for (uint32_t i = 0; i < _textureTypeFeatures.at(pipelineName).size(); ++i)
        layouts.push_back(_descriptorSets.layout(_textureFiles[0]));
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

ModelInstance &Model::instance(const std::string &instanceName) { return instance(instanceIndex(instanceName)); }

void Model::removeInstance(const std::string &instanceName) { _instanceIndexMap.erase(instanceName); }

const std::unordered_set<Model *> &Model::models() { return _models; }

} // namespace Modelbase