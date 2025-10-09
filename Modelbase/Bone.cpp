#include "AssimpGLMHelpers.h"
#include "Modelbase.h"
#include <glm/gtx/quaternion.hpp>

namespace Modelbase
{
Bone::Bone(const std::string &name, int id, const aiNodeAnim *channel) : _name(name), _id(id), _transform(1.0f)
{
    for (uint32_t i = 0; i < channel->mNumPositionKeys; ++i)
    {
        ModelData::KeyPosition position;
        position.position = AssimpGLMHelpers::getGLMVec3(channel->mPositionKeys[i].mValue);
        position.timeStamp = channel->mPositionKeys[i].mTime;
        _positions.push_back(position);
    }
    for (uint32_t i = 0; i < channel->mNumRotationKeys; ++i)
    {
        ModelData::KeyRotation rotation;
        rotation.rotation = AssimpGLMHelpers::getGLMQuat(channel->mRotationKeys[i].mValue);
        rotation.timeStamp = channel->mPositionKeys[i].mTime;
        _rotations.push_back(rotation);
    }
    for (uint32_t i = 0; i < channel->mNumScalingKeys; ++i)
    {
        ModelData::KeyScale scale;
        scale.scale = AssimpGLMHelpers::getGLMVec3(channel->mScalingKeys[i].mValue);
        scale.timeStamp = channel->mScalingKeys[i].mTime;
        _scales.push_back(scale);
    }
}

float Bone::scaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

glm::mat4 Bone::updatePosition(float animationTime)
{
    if (_positions.size() == 1)
        return glm::translate(glm::mat4(1.0f), _positions[0].position);
    try
    {
        int keyIndex = positionIndex(animationTime);
        glm::vec3 position = glm::mix(_positions[keyIndex].position, _positions[keyIndex + 1].position,
                                      scaleFactor(_positions[keyIndex].timeStamp, _positions[keyIndex + 1].timeStamp, animationTime));
        return glm::translate(glm::mat4(1.0f), position);
    }
    catch (std::runtime_error e)
    {
        return glm::translate(glm::mat4(1.0f), _positions[0].position);
    }
}

glm::mat4 Bone::updateRotation(float animationTime)
{
    if (_rotations.size() == 1)
        return glm::toMat4(glm::normalize(_rotations[0].rotation));
    try
    {
        int keyIndex = rotationIndex(animationTime);
        glm::quat rotation = glm::slerp(_rotations[keyIndex].rotation, _rotations[keyIndex + 1].rotation,
                                        scaleFactor(_rotations[keyIndex].timeStamp, _rotations[keyIndex + 1].timeStamp, animationTime));
        return glm::toMat4(rotation);
    }
    catch (std::runtime_error e)
    {
        return glm::toMat4(glm::normalize(_rotations[0].rotation));
    }
}

glm::mat4 Bone::updateScale(float animationTime)
{
    if (_scales.size() == 1)
        return glm::scale(glm::mat4(1.0f), _scales[0].scale);
    try
    {
        int keyIndex = scaleIndex(animationTime);
        glm::vec3 scale = glm::mix(_scales[keyIndex].scale, _scales[keyIndex + 1].scale,
                                   scaleFactor(_scales[keyIndex].timeStamp, _scales[keyIndex + 1].timeStamp, animationTime));
        return glm::scale(glm::mat4(1.0f), scale);
    }
    catch (std::runtime_error e)
    {
        return glm::scale(glm::mat4(1.0f), _scales[0].scale);
    }
}

int Bone::positionIndex(float animationTime)
{
    for (uint32_t i = 1; i < _positions.size(); ++i)
        if (animationTime < _positions[i].timeStamp)
            return i - 1;
    throw std::runtime_error("Failed to find the position index.");
}

int Bone::rotationIndex(float animationTime)
{
    for (uint32_t i = 1; i < _rotations.size(); ++i)
        if (animationTime < _rotations[i].timeStamp)
            return i - 1;
    throw std::runtime_error("Failed to find the rotation index.");
}

int Bone::scaleIndex(float animationTime)
{
    for (uint32_t i = 1; i < _scales.size(); ++i)
        if (animationTime < _scales[i].timeStamp)
            return i - 1;
    throw std::runtime_error("Failed to find the scale index.");
}

void Bone::update(float animationTime)
{
    glm::mat4 translation = updatePosition(animationTime);
    glm::mat4 rotation = updateRotation(animationTime);
    glm::mat4 scale = updateScale(animationTime);
    _transform = translation * rotation * scale;
}

const std::string &Bone::name() const { return _name; }

int Bone::id() const { return _id; }

glm::mat4 Bone::transform() const { return _transform; }
}; // namespace Modelbase