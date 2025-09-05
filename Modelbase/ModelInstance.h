#pragma once

#include "../Object/Object.h"
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

class ModelInstance
{
    friend class Model;

  private:
    float _animationProgress;
    bool _isAnimationIndexStackLock = false;
    Vkbase::DescriptorSets &_descriptorSets;
    std::vector<AnimationIndex> _animationIndexStack = std::vector<AnimationIndex>(1);
    Object _object;
    Model &_model;

    ModelInstance(const std::string &descriptorSetsName, const std::string &deviceName, Model &model);
    ~ModelInstance();
    void updateAnimation(float deltaTick);

  public:
    Object &object();
    Vkbase::DescriptorSets &descriptorSets();
    const Vkbase::DescriptorSets &descriptorSets() const;
    void updateUniformBuffers(uint32_t currentFrame, const Camera &camera);
    bool canAddAnimationToStack() const;
    void setBasicAnimation(const AnimationIndex &animationIndex);
    void addAnimationIndexToStack(const std::vector<AnimationIndex> &animationIndices);
};
} // namespace Modelbase
