#include "RenderObjectDelegator.h"
#include "../Camera/Camera.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "DescriptorSets.h"
#include "Pipeline.h"
namespace Vkbase
{
RenderObjectDelegator::RenderObjectDelegator(const std::string &deviceName, const Camera &camera, uint32_t flightFrameCount, vk::DeviceSize uboSize)
    : _flightFrameCount(flightFrameCount), _descriptorSetsName(createResource<DescriptorSets>("", deviceName).lock()->name()), _deviceName(deviceName),
      _pCamera(&camera)
{
    createUBOs(uboSize);
}

RenderObjectDelegator::RenderObjectDelegator(const std::string &deviceName, uint32_t flightFrameCount, vk::DeviceSize uboSize)
    : _flightFrameCount(flightFrameCount), _descriptorSetsName(createResource<DescriptorSets>("", deviceName).lock()->name()), _deviceName(deviceName)
{
    createUBOs(uboSize);
}

void Vkbase::RenderObjectDelegator::delegatorInit()
{
    auto descriptorSets = VkResourceManager::instance().resource(VkResourceType::DescriptorSets, _descriptorSetsName);
    if (auto p = descriptorSets.lock<DescriptorSets>())
    {
        addDescriptorSetsConfig(descriptorSets);
        p->init();
        writeDescriptorSets(descriptorSets);
    }
    else
        throw std::runtime_error("Failed to get the resource DescriptorSets.");
}

void RenderObjectDelegator::draw(const VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                                 uint32_t frameIndex) const
{
    onDraw(commandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);
}
void RenderObjectDelegator::update(uint32_t frameIndex) const { onUpdateUBO(frameIndex); }

uint32_t RenderObjectDelegator::flightFrameCount() const { return _flightFrameCount; }

void RenderObjectDelegator::createUBOs(vk::DeviceSize size)
{
    if (!size)
        return;

    _uboNames.reserve(_flightFrameCount);
    for (uint32_t i = 0; i < _flightFrameCount; ++i)
        _uboNames.push_back(createResource<Buffer>("", _deviceName, size, vk::BufferUsageFlagBits::eUniformBuffer).lock()->name());
}

void RenderObjectDelegator::writeUBODescriptorSets(const std::string &name, uint32_t binding) const
{
    std::vector<std::pair<vk::DescriptorBufferInfo, Vkbase::VkResourceManagerHolder::WeakReference>> bufferInfos;
    bufferInfos.reserve(_flightFrameCount);
    auto descriptorSets = VkResourceManager::instance().resource(VkResourceType::DescriptorSets, _descriptorSetsName);
    for (const std::string &uboName : _uboNames)
    {
        auto buffer = VkResourceManager::instance().resource(VkResourceType::Buffer, uboName);
        if (auto p = buffer.lock<Buffer>())
            bufferInfos.push_back({vk::DescriptorBufferInfo().setOffset(0).setRange(p->size()), buffer});
    }
    if (auto p = descriptorSets.lock<DescriptorSets>())
        p->writeSets(name, binding, bufferInfos, {}, bufferInfos.size());
}

void RenderObjectDelegator::writeUBODescriptorSets(const std::string &name, uint32_t binding, uint32_t firstIndex, uint32_t secondIndex) const
{
    if (firstIndex >= secondIndex)
        return;
    std::vector<std::pair<vk::DescriptorBufferInfo, VkResourceManagerHolder::WeakReference>> bufferInfos;
    bufferInfos.reserve(secondIndex - firstIndex);
    for (; firstIndex < secondIndex; ++firstIndex)
    {
        auto buffer = VkResourceManager::instance().resource(VkResourceType::Buffer, _uboNames[firstIndex]);
        if (auto p = buffer.lock<Buffer>())
            bufferInfos.push_back({vk::DescriptorBufferInfo().setOffset(0).setRange(p->size()), buffer});
    }
    if (auto p = VkResourceManager::instance().resource(VkResourceType::DescriptorSets, _descriptorSetsName).lock<DescriptorSets>())
        p->writeSets(name, binding, bufferInfos, {}, bufferInfos.size());
}

void RenderObjectDelegator::updateUBO(uint32_t frameIndex, const void *pData) const
{
    if (frameIndex >= _uboNames.size())
        throw std::out_of_range("onUpdateUBO: invalid frameIndex");

    if (auto p = VkResourceManager::instance().resource(VkResourceType::Buffer, _uboNames[frameIndex]).lock<Buffer>())
        p->updateBufferData(pData);
    else
        throw std::runtime_error("onUpdateUBO: missing UBO buffer");
}

const Camera &RenderObjectDelegator::camera() const
{
    if (!_pCamera)
        throw std::runtime_error("The Camera object didn't given in construct.");
    return *_pCamera;
}

const std::string &RenderObjectDelegator::descriptorSetsName() const { return _descriptorSetsName; }

const std::vector<std::string> &RenderObjectDelegator::uboNames() const { return _uboNames; }

const std::string &RenderObjectDelegator::deviceName() const { return _deviceName; }

} // namespace Vkbase