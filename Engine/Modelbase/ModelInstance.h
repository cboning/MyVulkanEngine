#pragma once

#include "../Object/Object.h"
#include "../Vkbase/VkResourcesDelegator.h"
#include <string>
#include <vector>

namespace Vkbase
{
class DescriptorSets;
}

class Camera;

namespace Modelbase
{

class Model;

struct AnimationIndex
{
    uint32_t animationIndex;
    float beginTime = 0.0f;
};

class ModelInstance : public Vkbase::VkResourcesDelegator
{
    friend class Model;

private:
    float _animationProgress;
    bool _isAnimationIndexStackLock = false;
    const Vkbase::VkResourceManagerHolder::WeakReference _descriptorSets;
    std::vector<AnimationIndex> _animationIndexStack = std::vector<AnimationIndex>(1);
    Object _object;
    Model &_model;

    ModelInstance(const std::string &deviceName, Model &model);
    ~ModelInstance() = default;
    void updateAnimation(float deltaTick);

public:
    Object &object();
    const Object &object() const;
    Vkbase::VkResourceManagerHolder::WeakReference descriptorSets() const;
    void updateUniformBuffers(uint32_t currentFrame, const Camera &camera) const;
    bool canAddAnimationToStack() const;
    void setBasicAnimation(const AnimationIndex &animationIndex);
    void addAnimationIndexToStack(const std::vector<AnimationIndex> &animationIndices);
};
} // namespace Modelbase
