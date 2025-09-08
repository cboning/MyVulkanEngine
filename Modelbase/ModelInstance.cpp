#include "ModelInstance.h"
#include "../Camera/Camera.h"
#include "../Vkbase/DescriptorSets.h"
#include "Animation.h"
#include "Model.h"
#include <iostream>

namespace Modelbase
{
ModelInstance::ModelInstance(const std::string &descriptorSetsName, const std::string &deviceName, Model &model)
    : _descriptorSets(*(Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>(descriptorSetsName, deviceName))), _model(model) {};

ModelInstance::~ModelInstance() { _descriptorSets.destroy(); }

Object &ModelInstance::object() { return _object; }

Vkbase::DescriptorSets &ModelInstance::descriptorSets() { return _descriptorSets; }

const Vkbase::DescriptorSets &ModelInstance::descriptorSets() const { return _descriptorSets; }

void ModelInstance::updateAnimation(float deltaTick)
{
    if (_animationIndexStack.size() < 2)
    {
        _animationProgress = _model.animation(_animationIndexStack[0].animationIndex).update(deltaTick);
        _isAnimationIndexStackLock = false;
        return;
    }

    Animation *pAnimation = &_model.animation(_animationIndexStack.back().animationIndex);

    deltaTick += _animationProgress;
    pAnimation->replay();

    while (pAnimation->willFinish(deltaTick) && _model.animationCount() > 1)
    {
        _animationIndexStack.pop_back();
        pAnimation = &_model.animation(_animationIndexStack.back().animationIndex);
        deltaTick += _animationIndexStack.back().beginTime;
        pAnimation->replay();
    }

    _animationProgress = pAnimation->update(deltaTick);
}

void ModelInstance::updateUniformBuffers(uint32_t currentFrame, const Camera &camera)
{
    ModelUniformData uniformData;
    if (_model.animationCount())
    {
        std::vector<glm::mat4> *transforms = _model.animation(_animationIndexStack.back().animationIndex).transformations();
        for (uint32_t i = 0; i < transforms->size(); ++i)
            uniformData.bonesMatrices[i] = (*transforms)[i];
    }

    uniformData.model = _object.matModel();
    uniformData.view = camera.view();
    uniformData.proj = camera.perspective();

    const Vkbase::Buffer &UBO = *dynamic_cast<const Vkbase::Buffer *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, _descriptorSets.name() + "_UBO_" + std::to_string(currentFrame)));
    UBO.updateBufferData(&uniformData);
}

bool ModelInstance::canAddAnimationToStack() const { return !_isAnimationIndexStackLock; }

void ModelInstance::setBasicAnimation(const AnimationIndex &animationIndex) { _animationIndexStack[0] = animationIndex; }

void ModelInstance::addAnimationIndexToStack(const std::vector<AnimationIndex> &animationIndices)
{
    if (!_isAnimationIndexStackLock)
    {
        _animationIndexStack.insert(_animationIndexStack.end(), animationIndices.begin(), animationIndices.end());
        _isAnimationIndexStackLock = true;
        return;
    }

    std::cout << "[Error Model] Can't add animation." << std::endl;
}

} // namespace Modelbase
