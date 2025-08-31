#include "AssimpGLMHelpers.h"
#include "Modelbase.h"
#include <iostream>

namespace Modelbase
{

Model::Model(const std::string &deviceName, const std::string &commandPoolName, vk::Sampler sampler, const std::string &fileName,
             const std::unordered_map<std::string, std::vector<aiTextureType>> &textureTypeFeatures,
             const std::unordered_map<std::string, std::string> &meshPipelineNames)
    : _device(*dynamic_cast<const Vkbase::Device *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Device, deviceName))),
      _commandPool(*dynamic_cast<const Vkbase::CommandPool *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::CommandPool, commandPoolName))),
      _sampler(sampler),
      _textureTypeFeatures(textureTypeFeatures),
      _meshPipelineNames(meshPipelineNames),
      _descriptorSets(*(new Vkbase::DescriptorSets(fileName, deviceName)))
{
    _models.insert(this);
    if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, "Empty"))
        new Vkbase::Image("Empty", deviceName, 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled,
                          (uint32_t[]){0xFFFF00FF});

    _textureFiles.push_back("Empty");

    loadModel(fileName);

    _descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, 0);
    applyTextureDescriptorSetConfig();
    _descriptorSets.init();

    writeTextureDescriptorSets(_sampler);
}

Model::Model(const std::string &deviceName, const std::string &commandPoolName, vk::Sampler sampler, json config)
    : Model(deviceName, commandPoolName, sampler, config["filename"], getTextureFeaturesWithConfig(config), getMeshToPipelineNamesWithConfig(config))
{
}

Model::~Model()
{
    _models.erase(this);
    for (uint32_t i = 0; i < _pAnimations.size(); ++i)
        delete _pAnimations[i];

    _descriptorSets.destroy();
}

void Model::loadModel(const std::string &fileName)
{
    const aiScene *pScene = _importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
        throw std::runtime_error("Failed to load model!");

    _fileDirectory = fileName.substr(0, fileName.find_last_of("/"));

    processNode(pScene->mRootNode, pScene, _rootNode);
    _animationCount = pScene->mNumAnimations;

    for (uint32_t i = 0; i < _animationCount; ++i)
    {
        _pAnimations.push_back(new Animation(pScene->mAnimations[i], this));
    }
}

void Model::processNode(aiNode *pNode, const aiScene *pScene, ModelData::AssimpNodeData &src)
{
    for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
    {
        aiMesh *pMesh = pScene->mMeshes[pNode->mMeshes[i]];
        processMesh(pMesh, pScene);
    }

    src.name = pNode->mName.data;
    src.transformation = AssimpGLMHelpers::getGLMMat4(pNode->mTransformation);
    src.childrenCount = pNode->mNumChildren;

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
    {
        ModelData::AssimpNodeData nodeData;
        processNode(pNode->mChildren[i], pScene, nodeData);
        src.children.push_back(nodeData);
    }
}

void Model::initVertexBoneData(ModelData::Vertex &vertex)
{
    for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        vertex.boneIds[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}

void Model::setVertexBoneData(ModelData::Vertex &vertex, int boneId, float weight)
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

void Model::loadVerticesBoneWeight(std::vector<ModelData::Vertex> &vertices, aiMesh *pMesh)
{
    for (uint32_t boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex)
    {
        int boneId;
        std::string boneName = pMesh->mBones[boneIndex]->mName.C_Str();
        if (_boneInfoMap.find(boneName) == _boneInfoMap.end())
        {
            ModelData::BoneInfo boneInfo;
            boneInfo.id = _boneCount;
            boneInfo.offset = AssimpGLMHelpers::getGLMMat4(pMesh->mBones[boneIndex]->mOffsetMatrix);
            _boneInfoMap[boneName] = boneInfo;
            boneId = _boneCount++;
        }
        else
            boneId = _boneInfoMap[boneName].id;

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

void Model::createDescriptorSets(uint32_t instanceIndex)
{
    Instance &animationInstance = _instances[instanceIndex];

    addUBODescriptorSetsConfig(instanceIndex);
    animationInstance.descriptorSets.init();
    writeDescriptorSets(instanceIndex);
}

void Model::processMesh(aiMesh *pMesh, const aiScene *pScene)
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
    loadVerticesBoneWeight(vertices, pMesh);

    for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
    {
        aiFace face = pMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    std::string meshName(pMesh->mName.C_Str());
    std::string pipelineName;
    if (_meshPipelineNames.count(meshName))
        pipelineName = _meshPipelineNames.at(meshName);
    else
        pipelineName = _meshPipelineNames.at("default");

    const std::vector<aiTextureType> textureTypeFeatures = _textureTypeFeatures.at(pipelineName);
    aiMaterial *pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
    std::vector<std::vector<std::string>> textureNames;
    textureNames.resize(textureTypeFeatures.size(), {"Empty"});

    for (uint32_t i = 0; i < textureTypeFeatures.size(); ++i)
        textureNames[i][0] = loadMaterialTextures(pMaterial, textureTypeFeatures[i])[0];
    _meshs.emplace_back(_descriptorSets.name() + "_" + pMesh->mName.C_Str(), _device.name(), vertices, indices, textureNames);
}

std::vector<std::string> Model::loadMaterialTextures(aiMaterial *pMaterial, aiTextureType textureType)
{
    std::vector<std::string> textureNames;
    for (unsigned int i = 0; i < pMaterial->GetTextureCount(textureType); ++i)
    {
        aiString path;
        pMaterial->GetTexture(textureType, i, &path);
        std::string filename = _fileDirectory + "/" + path.C_Str();
        if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, filename))
        {
            new Vkbase::Image(filename, _device.name(), filename, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D,
                              vk::ImageUsageFlagBits::eSampled);
            _textureFiles.push_back(filename);
        }
        textureNames.push_back(filename);
    }
    if (textureNames.empty())
        textureNames.push_back("Empty");
    return textureNames;
}

void Model::draw(uint32_t currentFrame, const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, uint32_t instanceIndex)
{
    for (Mesh<ModelData::Vertex> &mesh : _meshs)
    {
        std::vector<vk::DescriptorSet> descriptorSets;
        descriptorSets.push_back(_instances[instanceIndex].descriptorSets.sets("UBO")[currentFrame]);
        const std::vector<std::vector<std::string>> &textureNames = mesh.textureNames();

        for (const std::vector<std::string> &textureName : textureNames)
            descriptorSets.push_back(_descriptorSets.sets(textureName[0])[0]);

        mesh.draw(commandBuffer, pipeline, descriptorSets);
    }
}

void Model::updateAnimation(uint32_t instanceIndex, float deltaTick)
{
    if (_animationCount < 1)
        return;

    if (instanceIndex >= _instances.size())
    {
        std::cout << "[Error Model] Can't find the object." << std::endl;
        return;
    }

    Instance &animationInstance = _instances[instanceIndex];

    if (animationInstance.animationIndexStack.size() < 2)
    {
        animationInstance.animationProgress = _pAnimations[animationInstance.animationIndexStack[0].animationIndex]->update(deltaTick);
        animationInstance.isAnimationIndexStackLock = false;
        return;
    }

    Animation *pAnimation = _pAnimations[animationInstance.animationIndexStack.back().animationIndex];

    deltaTick += animationInstance.animationProgress;
    pAnimation->replay();

    while (pAnimation->willFinish(deltaTick) && _pAnimations.size() > 1)
    {
        animationInstance.animationIndexStack.pop_back();
        pAnimation = _pAnimations[animationInstance.animationIndexStack.back().animationIndex];
        deltaTick += animationInstance.animationIndexStack.back().beginTime;
        pAnimation->replay();
    }

    animationInstance.animationProgress = pAnimation->update(deltaTick);
}

void Model::updateAnimation(const std::string &instanceName, float deltaTick)
{
    updateAnimation(instanceIndex(instanceName), deltaTick);
}

void Model::updateAnimation(float deltaTick)
{
    for (auto instanceIndex : _instanceIndexMap)
        updateAnimation(instanceIndex.second, deltaTick);
}

void Model::updateUniformBuffers(uint32_t instanceIndex, uint32_t currentFrame, const Camera &camera)
{
    Instance &instance = _instances[instanceIndex];
    ModelUniformData uniformData;
    if (_animationCount)
    {
        std::vector<glm::mat4> *transforms = _pAnimations[instance.animationIndexStack.back().animationIndex]->transformations();
        for (uint32_t i = 0; i < transforms->size(); ++i)
            uniformData.bonesMatrices[i] = (*transforms)[i];
    }

    uniformData.model = instance.object.matModel();
    uniformData.view = camera.view();
    uniformData.proj = camera.perspective();

    const Vkbase::Buffer &UBO = *dynamic_cast<const Vkbase::Buffer *>(Vkbase::ResourceBase::resourceManager().resource(
        Vkbase::ResourceType::Buffer, _instances[instanceIndex].descriptorSets.name() + "_UBO_" + std::to_string(currentFrame)));
    UBO.updateBufferData(&uniformData);
}

void Model::updateUniformBuffers(const std::string &instanceName, uint32_t currentFrame, const Camera &camera)
{
    updateUniformBuffers(instanceIndex(instanceName), currentFrame, camera);
}

void Model::addAnimationIndexToStack(uint32_t instanceIndex, std::vector<AnimationIndex> animationIndices)
{
    Instance &animationInstance = _instances[instanceIndex];
    if (!animationInstance.isAnimationIndexStackLock)
    {
        animationInstance.animationIndexStack.insert(animationInstance.animationIndexStack.end(), animationIndices.begin(), animationIndices.end());
        animationInstance.isAnimationIndexStackLock = true;
        return;
    }

    std::cout << "[Error Model] Can't add animation." << std::endl;
}

void Model::addAnimationIndexToStack(const std::string &instanceName, std::vector<AnimationIndex> animationIndices)
{
    addAnimationIndexToStack(instanceIndex(instanceName), animationIndices);
}

void Model::setBasicAnimation(uint32_t instanceIndex, uint32_t animationIndex)
{
    _instances[instanceIndex].animationIndexStack[0].animationIndex = animationIndex;
}

void Model::setBasicAnimation(const std::string &instanceName, uint32_t animationIndex)
{
    setBasicAnimation(instanceIndex(instanceName), animationIndex);
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

void Model::addUBODescriptorSetsConfig(uint32_t instanceIndex)
{
    Instance &animationInstance = _instances[instanceIndex];
    animationInstance.descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}},
                                                                  MAX_FLIGHT_COUNT,
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

void Model::writeDescriptorSets(uint32_t instanceIndex)
{
    Instance &animationInstance = _instances[instanceIndex];
    std::vector<vk::DescriptorBufferInfo> bufferInfos(MAX_FLIGHT_COUNT, vk::DescriptorBufferInfo().setOffset(0).setRange(sizeof(ModelUniformData)));
    uint32_t count = 0;
    for (vk::DescriptorBufferInfo &bufferInfo : bufferInfos)
    {
        bufferInfo.setBuffer((new Vkbase::Buffer(animationInstance.descriptorSets.name() + "_UBO_" + std::to_string(count), _device.name(),
                                                 sizeof(ModelUniformData), vk::BufferUsageFlagBits::eUniformBuffer))
                                 ->buffer());
        ++count;
    }

    animationInstance.descriptorSets.writeSets("UBO", 0, bufferInfos, {}, MAX_FLIGHT_COUNT);
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
    if (textureType == "NONE")
        return aiTextureType_NONE;
    else if (textureType == "DIFFUSE")
        return aiTextureType_DIFFUSE;
    else if (textureType == "SPECULAR")
        return aiTextureType_SPECULAR;
    else if (textureType == "AMBIENT")
        return aiTextureType_AMBIENT;
    else if (textureType == "EMISSIVE")
        return aiTextureType_EMISSIVE;
    else if (textureType == "HEIGHT")
        return aiTextureType_HEIGHT;
    else if (textureType == "NORMALS")
        return aiTextureType_NORMALS;
    else if (textureType == "SHININESS")
        return aiTextureType_SHININESS;
    else if (textureType == "OPACITY")
        return aiTextureType_OPACITY;
    else if (textureType == "DISPLACEMENT")
        return aiTextureType_DISPLACEMENT;
    else if (textureType == "LIGHTMAP")
        return aiTextureType_LIGHTMAP;
    else if (textureType == "REFLECTION")
        return aiTextureType_REFLECTION;
    else if (textureType == "BASE_COLOR")
        return aiTextureType_BASE_COLOR;
    else if (textureType == "NORMAL_CAMERA")
        return aiTextureType_NORMAL_CAMERA;
    else if (textureType == "EMISSION_COLOR")
        return aiTextureType_EMISSION_COLOR;
    else if (textureType == "METALNESS")
        return aiTextureType_METALNESS;
    else if (textureType == "DIFFUSE_ROUGHNESS")
        return aiTextureType_DIFFUSE_ROUGHNESS;
    else if (textureType == "AMBIENT_OCCLUSION")
        return aiTextureType_AMBIENT_OCCLUSION;
    else if (textureType == "SHEEN")
        return aiTextureType_SHEEN;
    else if (textureType == "CLEARCOAT")
        return aiTextureType_CLEARCOAT;
    else if (textureType == "TRANSMISSION")
        return aiTextureType_TRANSMISSION;
    else if (textureType == "MAYA_BASE")
        return aiTextureType_MAYA_BASE;
    else if (textureType == "MAYA_SPECULAR")
        return aiTextureType_MAYA_SPECULAR;
    else if (textureType == "MAYA_SPECULAR_COLOR")
        return aiTextureType_MAYA_SPECULAR_COLOR;
    else if (textureType == "MAYA_SPECULAR_ROUGHNESS")
        return aiTextureType_MAYA_SPECULAR_ROUGHNESS;
    else if (textureType == "ANISOTROPY")
        return aiTextureType_ANISOTROPY;
    else if (textureType == "GLTF_METALLIC_ROUGHNESS")
        return aiTextureType_GLTF_METALLIC_ROUGHNESS;
    else if (textureType == "UNKNOWN")
        return aiTextureType_UNKNOWN;
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

void Model::createNewInstance(const std::string &instanceName, AnimationIndex basedAnimationIndex)
{
    if (_instanceIndexMap.find(instanceName) != _instanceIndexMap.end())
    {
        std::cout << "[Error Model] There is a instance was created, it's name is " << instanceName << ", so you can't create a new instance have the same name"
                  << std::endl;
        return;
    }

    if (!_removedInstanceIndices.empty())
    {
        _instanceIndexMap[instanceName] = _removedInstanceIndices.back();
        _removedInstanceIndices.pop_back();

        Instance &animationInstance = _instances[_instanceIndexMap[instanceName]];

        animationInstance.animationIndexStack.resize(1);
        animationInstance.animationIndexStack[0] = basedAnimationIndex;
        animationInstance.animationProgress = 0.0f;
        return;
    }

    _instanceIndexMap[instanceName] = _instances.size();
    _instances.emplace_back("Model_" + instanceName, _device.name());

    Instance &animationInstance = _instances[_instanceIndexMap[instanceName]];

    createDescriptorSets(_instanceIndexMap[instanceName]);

    animationInstance.animationIndexStack.resize(1);
    animationInstance.animationIndexStack[0] = basedAnimationIndex;
    animationInstance.animationProgress = 0.0f;
}

std::unordered_map<std::string, ModelData::BoneInfo> &Model::boneInfoMap()
{
    return _boneInfoMap;
}

int &Model::boneCount()
{
    return _boneCount;
}

ModelData::AssimpNodeData *Model::rootNode()
{
    return &_rootNode;
}

std::vector<vk::DescriptorSetLayout> Model::descriptorSetLayout(uint32_t instanceIndex, const std::string &pipelineName) const
{
    std::vector<vk::DescriptorSetLayout> layouts;
    layouts.push_back(_instances[instanceIndex].descriptorSets.layout("UBO"));
    for (uint32_t i = 0; i < _textureTypeFeatures.at(pipelineName).size(); ++i)
        layouts.push_back(_descriptorSets.layout(_textureFiles[0]));
    return layouts;
}

std::vector<vk::DescriptorSetLayout> Model::descriptorSetLayout(const std::string &instanceName, const std::string &pipelineName) const
{
    return descriptorSetLayout(instanceIndex(instanceName), pipelineName);
}

uint32_t Model::animationCount()
{
    return _animationCount;
}

bool Model::canAddAnimation(uint32_t instanceIndex)
{
    return !_instances[instanceIndex].isAnimationIndexStackLock;
}

bool Model::canAddAnimation(const std::string &instanceName)
{
    return canAddAnimation(instanceIndex(instanceName));
}

int32_t Model::instanceIndex(const std::string &instanceName) const
{
    if (_instanceIndexMap.find(instanceName) == _instanceIndexMap.end())
    {
        std::cout << "[Model Error] There is not instance name is " << instanceName << std::endl;
        return -1;
    }

    return _instanceIndexMap.at(instanceName);
}

Object &Model::instanceObject(uint32_t instanceIndex)
{
    return _instances[instanceIndex].object;
}

Object &Model::instanceObject(const std::string &instanceName)
{
    return instanceObject(instanceIndex(instanceName));
}

void Model::removeInstance(const std::string &instanceName)
{
    _removedInstanceIndices.push_back(instanceIndex(instanceName));
    _instanceIndexMap.erase(instanceName);
}

const std::unordered_set<Model *> &Model::models()
{
    return _models;
}

} // namespace Modelbase