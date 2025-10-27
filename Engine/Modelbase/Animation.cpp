#include "Modelbase.h"
namespace Modelbase
{
    Animation::Animation(aiAnimation *pAnimation, Model &model)
        : _model(model)
    {
        _duration = pAnimation->mDuration;
        _ticksPerSecond = pAnimation->mTicksPerSecond;

        std::unordered_map<std::string, ModelData::BoneInfo> &boneInfoMap = model.boneInfoMap();
        int &boneCount = model.boneCount();
        for (uint32_t i = 0; i < pAnimation->mNumChannels; ++i)
        {
            aiNodeAnim *pChannel = pAnimation->mChannels[i];
            if (boneInfoMap.find(pChannel->mNodeName.data) == boneInfoMap.end())
            {
                boneInfoMap[pChannel->mNodeName.data].id = boneCount++;
            }
            _pBones.push_back(new Bone(pChannel->mNodeName.data, boneInfoMap[pChannel->mNodeName.data].id, pChannel));
        }

        _transforms.reserve(boneInfoMap.size());
        for (uint32_t i = 0; i < boneInfoMap.size(); ++i)
            _transforms.push_back(glm::mat4(1.0f));
    }

    float Animation::update(float deltaTime)
    {
        _currentTime += deltaTime * _ticksPerSecond;
        _currentTime = fmod(_currentTime, _duration);
        calculateBoneTransform(_model.rootNode(), glm::mat4(1.0f));
        return _currentTime;
    }

    void Animation::calculateBoneTransform(const ModelData::AssimpNodeData *pNode, glm::mat4 parentTransform)
    {
        Bone *pBone = findBone(pNode->name);
        if (pBone)
        {
            pBone->update(_currentTime);
            parentTransform *= pBone->transform();
        }
        else
            parentTransform *= pNode->transformation;

        std::unordered_map<std::string, ModelData::BoneInfo> &boneInfoMap = _model.boneInfoMap();

        if (boneInfoMap.find(pNode->name) != boneInfoMap.end())
            _transforms[boneInfoMap[pNode->name].id] = parentTransform * boneInfoMap[pNode->name].offset;

        for (int i = 0; i < pNode->childrenCount; ++i)
            calculateBoneTransform(&pNode->children[i], parentTransform);
    }

    void Animation::replay()
    {
        _currentTime = 0.0f;
    }

    Bone *Animation::findBone(const std::string &name)
    {
        std::vector<Bone *>::iterator iter = find_if(_pBones.begin(), _pBones.end(), [&](Bone *pBone)
                                                     { return pBone->name() == name; });
        if (iter == _pBones.end())
            return nullptr;
        else
            return *iter;
    }

    std::vector<glm::mat4> *Animation::transformations()
    {
        return &_transforms;
    }

    bool Animation::willFinish(float &deltaTime)
    {
        deltaTime = _currentTime + deltaTime - _duration;
        return deltaTime > 0;
    }
};