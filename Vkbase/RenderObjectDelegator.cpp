#include "RenderObjectDelegator.h"
#include "../Camera/Camera.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "DescriptorSets.h"
#include "Pipeline.h"
namespace Vkbase
{
RenderObjectDelegator::RenderObjectDelegator(const std::string &deviceName, const Camera &camera, uint32_t flightFrameCount, vk::DeviceSize uboSize)
    : _flightFrameCount(flightFrameCount), _descriptorSetsName(createResource<DescriptorSets>("", deviceName)->name()), _deviceName(deviceName), _camera(camera)
{
    createUBOs(uboSize);
}

void Vkbase::RenderObjectDelegator::delegatorInit()
{
    DescriptorSets *pDescriptorSets =
        dynamic_cast<DescriptorSets *>(VkResourceManager::instance().resource(VkResourceType::DescriptorSets, _descriptorSetsName));
    if (!pDescriptorSets)
        throw std::runtime_error("Failed to get the resource DescriptorSets.");
    addDescriptorSetsConfig(*pDescriptorSets);
    pDescriptorSets->init();
    writeDescriptorSets(*pDescriptorSets);
}

void RenderObjectDelegator::draw(CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex, uint32_t frameIndex) const
{
    onDraw(pCommandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);
}
void RenderObjectDelegator::update(uint32_t frameIndex) const { onUpdateUBO(frameIndex); }

uint32_t RenderObjectDelegator::flightFrameCount() const { return _flightFrameCount; }

void RenderObjectDelegator::createUBOs(vk::DeviceSize size)
{
    if (!size)
        return;

    _uboNames.reserve(_flightFrameCount);
    for (uint32_t i = 0; i < _flightFrameCount; ++i)
        _uboNames.push_back(createResource<Buffer>("", _deviceName, size, vk::BufferUsageFlagBits::eUniformBuffer)->name());
}

void RenderObjectDelegator::writeUBODescriptorSets(const std::string &name, uint32_t binding) const
{
    std::vector<std::pair<vk::DescriptorBufferInfo, Buffer *>> bufferInfos;
    bufferInfos.reserve(_flightFrameCount);
    DescriptorSets *pDescriptorSets =
        dynamic_cast<DescriptorSets *>(VkResourceManager::instance().resource(VkResourceType::DescriptorSets, _descriptorSetsName));
    for (const std::string &uboName : _uboNames)
    {
        Buffer *pBuffer = dynamic_cast<Buffer *>(VkResourceManager::instance().resource(VkResourceType::Buffer, uboName));
        bufferInfos.push_back({vk::DescriptorBufferInfo().setOffset(0).setRange(pBuffer->size()), pBuffer});
    }
    pDescriptorSets->writeSets(name, binding, bufferInfos, {}, bufferInfos.size());
}

void RenderObjectDelegator::writeUBODescriptorSets(const std::string &name, uint32_t binding, uint32_t firstIndex, uint32_t secondIndex) const
{
    if (firstIndex >= secondIndex)
        return;
    std::vector<std::pair<vk::DescriptorBufferInfo, Buffer *>> bufferInfos;
    bufferInfos.reserve(secondIndex - firstIndex);
    DescriptorSets *pDescriptorSets =
        dynamic_cast<DescriptorSets *>(VkResourceManager::instance().resource(VkResourceType::DescriptorSets, _descriptorSetsName));
    for (; firstIndex < secondIndex; ++firstIndex)
    {
        Buffer *pBuffer = dynamic_cast<Buffer *>(VkResourceManager::instance().resource(VkResourceType::Buffer, _uboNames[firstIndex]));
        bufferInfos.push_back({vk::DescriptorBufferInfo().setOffset(0).setRange(pBuffer->size()), pBuffer});
    }
    pDescriptorSets->writeSets(name, binding, bufferInfos, {}, bufferInfos.size());
}

void RenderObjectDelegator::updateUBO(uint32_t frameIndex, const void *pData) const
{
    if (frameIndex >= _uboNames.size())
        throw std::out_of_range("onUpdateUBO: invalid frameIndex");

    Buffer *pBuffer = dynamic_cast<Buffer *>(VkResourceManager::instance().resource(VkResourceType::Buffer, _uboNames[frameIndex]));
    if (!pBuffer)
        throw std::runtime_error("onUpdateUBO: missing UBO buffer");

    pBuffer->updateBufferData(pData);
}

const Camera &RenderObjectDelegator::camera() const { return _camera; }

const std::string &RenderObjectDelegator::descriptorSetsName() const { return _descriptorSetsName; }

const std::vector<std::string> &RenderObjectDelegator::uboNames() const { return _uboNames; }

const std::string &RenderObjectDelegator::deviceName() const { return _deviceName; }

} // namespace Vkbase