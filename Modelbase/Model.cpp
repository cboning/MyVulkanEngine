#include "Modelbase.h"
#include "AssimpGLMHelpers.h"
#include <iostream>
template class Modelbase::Mesh<ModelData::Vertex>;

namespace Modelbase
{
    Model::Model(const std::string &deviceName, const std::string &commandPoolName, const std::string &fileName, const std::vector<aiTextureType> &textureTypeFeatures, vk::Sampler sampler)
        : _device(*dynamic_cast<const Vkbase::Device *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Device, deviceName))), _commandPool(*dynamic_cast<const Vkbase::CommandPool *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::CommandPool, commandPoolName))), _sampler(sampler), _textureTypeFeatures(textureTypeFeatures), _descriptorSets(*(new Vkbase::DescriptorSets(fileName, deviceName)))
    {
        _models.insert(this);
        if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, "Empty"))
            new Vkbase::Image("Empty", deviceName, 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled, (uint32_t[]){0xFFFF00FF});

        _files.push_back("Empty");

        Vkbase::Sampler *pSampler = new Vkbase::Sampler("Sampler", "0");

        loadModel(fileName);

        _descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, 0);

        applyTextureDescriptorSetConfig();
        _descriptorSets.init();
        writeTextureDescriptorSets(pSampler->sampler());
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
        AnimationInstance &animationInstance = _animationInstances[instanceIndex];

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

        aiMaterial *pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
        std::vector<std::vector<std::string>> textureNames;
        textureNames.resize(_textureTypeFeatures.size(), {"Empty"});

        for (uint32_t i = 0; i < _textureTypeFeatures.size(); ++i)
            textureNames[i][0] = loadMaterialTextures(pMaterial, _textureTypeFeatures[i])[0];
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
                new Vkbase::Image(filename, _device.name(), filename, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled);
                _files.push_back(filename);
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
            descriptorSets.push_back(_animationInstances[instanceIndex].descriptorSets.sets("UBO")[currentFrame]);
            const std::vector<std::vector<std::string>> &textureNames = mesh.textureNames();

            for (const std::vector<std::string> &textureName : textureNames)
                descriptorSets.push_back(_descriptorSets.sets(textureName[0])[0]);
            
            mesh.draw(commandBuffer, pipeline, descriptorSets);
        }
    }

    void Model::updateAnimation(uint32_t instanceIndex, float deltaTime)
    {
        if (_animationCount < 1)
            return ;

        if (instanceIndex >= _animationInstances.size())
        {
            std::cout << "[Error Model] Can't find the object." << std::endl;
            return;
        }

        AnimationInstance &animationInstance = _animationInstances[instanceIndex];

        if (animationInstance.animationIndexStack.size() < 2)
        {
            animationInstance.animationProgress = _pAnimations[animationInstance.animationIndexStack[0].animationIndex]->update(deltaTime);
            animationInstance.isAnimationIndexStackLock = false;
            return;
        }

        Animation *pAnimation = _pAnimations[animationInstance.animationIndexStack.back().animationIndex];

        deltaTime += animationInstance.animationProgress;
        pAnimation->replay();

        while (pAnimation->willFinish(deltaTime) && _pAnimations.size() > 1)
        {
            animationInstance.animationIndexStack.pop_back();
            pAnimation = _pAnimations[animationInstance.animationIndexStack.back().animationIndex];
            deltaTime += animationInstance.animationIndexStack.back().beginTime;
            pAnimation->replay();
        }

        animationInstance.animationProgress = pAnimation->update(deltaTime);
    }

    void Model::updateAnimation(const std::string &instanceName, float deltaTime)
    {
        updateAnimation(instanceIndex(instanceName), deltaTime);
    }

    void Model::updateUniformBuffers(uint32_t instanceIndex, uint32_t currentFrame, const Camera &camera)
    {
        AnimationInstance &instance = _animationInstances[instanceIndex];
        ModelUniformData uniformData;
        if (_animationCount)
        {
            std::vector<glm::mat4> *transforms = _pAnimations[instance.animationIndexStack.back().animationIndex]->transformations();
            for (uint32_t i = 0; i < transforms->size(); ++i)
                uniformData.bonesMatrices[i] = (*transforms)[i];
        }

        uniformData.model = glm::translate(glm::scale(glm::toMat4(instance.rotate), instance.scale), instance.position);

        uniformData.view = camera.view();
        uniformData.proj = camera.perspective();

        const Vkbase::Buffer &UBO = *dynamic_cast<const Vkbase::Buffer *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, _animationInstances[instanceIndex].descriptorSets.name() + "_UBO_" + std::to_string(currentFrame)));
        UBO.updateBufferData(&uniformData);
    }

    void Model::updateUniformBuffers(const std::string &instanceName, uint32_t currentFrame, const Camera &camera)
    {
        updateUniformBuffers(instanceIndex(instanceName), currentFrame, camera);
    }

    void Model::addAnimationIndexToStack(uint32_t instanceIndex, std::vector<AnimationIndex> animationIndices)
    {
        AnimationInstance &animationInstance = _animationInstances[instanceIndex];
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
        _animationInstances[instanceIndex].animationIndexStack[0].animationIndex = animationIndex;
    }

    void Model::setBasicAnimation(const std::string &instanceName, uint32_t animationIndex)
    {
        setBasicAnimation(instanceIndex(instanceName), animationIndex);
    }

    void Model::applyTextureDescriptorSetConfig()
    {
        uint32_t count = 0;
        for (const std::string &file : _files)
        {
            _descriptorSets.addDescriptorSetCreateConfig(file, {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, 1, count ? std::pair<const Vkbase::DescriptorSets *, std::string>{&_descriptorSets, _files[0]} : std::pair<const Vkbase::DescriptorSets *, std::string>{nullptr, ""});
            ++count;
        }
    }

    void Model::addUBODescriptorSetsConfig(uint32_t instanceIndex)
    {
        AnimationInstance &animationInstance = _animationInstances[instanceIndex];
        animationInstance.descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, MAX_FLIGHT_COUNT, std::pair<const Vkbase::DescriptorSets *, std::string>{&_descriptorSets, "UBO"});
    }

    void Model::writeTextureDescriptorSets(const vk::Sampler &sampler) const
    {
        std::vector<vk::DescriptorImageInfo> imageInfo(1);
        imageInfo[0].setSampler(sampler);
        for (const std::string &file : _files)
        {
            const Vkbase::Image &image = *dynamic_cast<const Vkbase::Image *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, file));
            imageInfo[0].setImageView(image.view())
                .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            _descriptorSets.writeSets(file, 0, {}, imageInfo, 1);
        }
    }

    void Model::writeDescriptorSets(uint32_t instanceIndex)
    {
        AnimationInstance &animationInstance = _animationInstances[instanceIndex];
        std::vector<vk::DescriptorBufferInfo> bufferInfos(MAX_FLIGHT_COUNT, vk::DescriptorBufferInfo().setOffset(0).setRange(sizeof(ModelUniformData)));
        uint32_t count = 0;
        for (vk::DescriptorBufferInfo &bufferInfo : bufferInfos)
        {
            bufferInfo.setBuffer((new Vkbase::Buffer(animationInstance.descriptorSets.name() + "_UBO_" + std::to_string(count), _device.name(), sizeof(ModelUniformData), vk::BufferUsageFlagBits::eUniformBuffer))->buffer());
            ++count;
        }
            
        animationInstance.descriptorSets.writeSets("UBO", 0, bufferInfos, {}, MAX_FLIGHT_COUNT);
    }

    void Model::createNewInstance(const std::string &instanceName, AnimationIndex basedAnimationIndex)
    {
        if (_instanceIndexMap.find(instanceName) != _instanceIndexMap.end())
        {
            std::cout << "[Error Model] There is a instance was created, it's name is " << instanceName << ", so you can't create a new instance have the same name" << std::endl;
            return ;
        }

        if (!_removedInstanceIndices.empty())
        {
            _instanceIndexMap[instanceName] = _removedInstanceIndices.back();
            _removedInstanceIndices.pop_back();

            AnimationInstance &animationInstance = _animationInstances[_instanceIndexMap[instanceName]];

            animationInstance.animationIndexStack.resize(1);
            animationInstance.animationIndexStack[0] = basedAnimationIndex;
            animationInstance.animationProgress = 0.0f;
            return;
        }

        _instanceIndexMap[instanceName] = _animationInstances.size();
        _animationInstances.emplace_back("Model_" + instanceName, _device.name());

        AnimationInstance &animationInstance = _animationInstances[_instanceIndexMap[instanceName]];

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

    std::vector<vk::DescriptorSetLayout> Model::descriptorSetLayout(uint32_t instanceIndex) const
    {
        std::vector<vk::DescriptorSetLayout> layout;
        layout.push_back(_animationInstances[instanceIndex].descriptorSets.layout("UBO"));
        for (uint32_t i = 0; i < _textureTypeFeatures.size(); ++i)
            layout.push_back(_descriptorSets.layout(_files[0]));
        return layout;
    }

    std::vector<vk::DescriptorSetLayout> Model::descriptorSetLayout(const std::string &instanceName) const
    {
        return descriptorSetLayout(instanceIndex(instanceName));
    }

    uint32_t Model::animationCount()
    {
        return _animationCount;
    }

    bool Model::canAddAnimation(uint32_t instanceIndex)
    {
        return !_animationInstances[instanceIndex].isAnimationIndexStackLock;
    }

    bool Model::canAddAnimation(const std::string &instanceName)
    {
        return canAddAnimation(instanceIndex(instanceName));
    }

    uint32_t Model::instanceIndex(const std::string &instanceName) const
    {
        if (_instanceIndexMap.find(instanceName) == _instanceIndexMap.end())
        {
            std::cout << "[Model Error] There is not instance name is " << instanceName << std::endl;
            return -1;
        }

        return _instanceIndexMap.at(instanceName);
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
};