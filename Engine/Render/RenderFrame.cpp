#include "RenderFrame.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/CommandBuffer.h"
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Device.h"
#include "../Vkbase/Pipeline.h"
#include "../Vkbase/VkResourceManager.h"

RenderFrame::RenderFrame(const std::string &deviceName,
                         const std::vector<std::vector<std::pair<std::string, std::pair<std::string, uint32_t>>>> &descriptorSets)
    : RenderObjectDelegator(deviceName, 0, 0), _descriptorSets(descriptorSets)
{
    ScreenVertexData frameVertices[] = {{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},  {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}};
    _frameVerticesBuffer = createResource<Vkbase::Buffer>("", deviceName, sizeof(ScreenVertexData) * 6, vk::BufferUsageFlagBits::eVertexBuffer, frameVertices);
}

void RenderFrame::onDraw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &, const std::string &, uint32_t imageIndex,
                         uint32_t) const
{
    if (auto pCommandBuffer = commandBuffer.lock<Vkbase::CommandBuffer>())
    {
        if (auto pFrameVerticesBuffer = _frameVerticesBuffer.lock<Vkbase::Buffer>())
            if (pCommandBuffer->device() != pFrameVerticesBuffer->device())
                throw std::runtime_error("Device mismatch");
        std::vector<std::pair<Vkbase::VkResourceManagerHolder::WeakReference, std::pair<std::string, uint32_t>>> descriptorSets;
        descriptorSets.reserve(_descriptorSets[imageIndex].size());
        for (auto descriptorSet : _descriptorSets[imageIndex])
        {
            descriptorSets.push_back(
                {Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::DescriptorSets, descriptorSet.first), descriptorSet.second});
        }
        pCommandBuffer->bindDescriptorSets(0, descriptorSets, {});
        pCommandBuffer->bindVertexBuffers(0, {_frameVerticesBuffer}, {0});
        pCommandBuffer->commandBuffer().draw(6, 1, 0, 0);
    }
}
void RenderFrame::onUpdateUBO(uint32_t) const {}

void RenderFrame::addDescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &) {}

void RenderFrame::writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &) {}