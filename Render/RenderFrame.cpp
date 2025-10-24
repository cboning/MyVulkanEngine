#include "RenderFrame.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/CommandBuffer.h"
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Device.h"
#include "../Vkbase/Pipeline.h"
#include "../Vkbase/VkResourceManager.h"

RenderFrame::RenderFrame(const std::string &deviceName,
                         const std::vector<std::vector<std::pair<std::string, std::pair<std::string, uint32_t>>>> &descriptorSets)
    : RenderObjectDelegator(deviceName, *(const Camera *)(nullptr), 0, 0), _descriptorSets(descriptorSets)
{
    ScreenVertexData frameVertices[] = {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}, {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
                                        {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
                                        {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}};
    _pFrameVerticesBuffer = createResource<Vkbase::Buffer>("", deviceName, sizeof(ScreenVertexData) * 6, vk::BufferUsageFlagBits::eVertexBuffer, frameVertices);
}

void RenderFrame::onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &, const std::string &pipelineName, uint32_t imageIndex,
                         uint32_t frameIndex) const
{
    if (pCommandBuffer->device().device() != _pFrameVerticesBuffer->device().device())
        throw std::runtime_error("Device mismatch");
    std::vector<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> descriptorSets;
    descriptorSets.reserve(_descriptorSets[imageIndex].size());
    for (auto descriptorSet : _descriptorSets[imageIndex])
    {
        descriptorSets.push_back({dynamic_cast<Vkbase::DescriptorSets *>(
                                      Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::DescriptorSets, descriptorSet.first)),
                                  descriptorSet.second});

    }
    pCommandBuffer->bindDescriptorSets(0, descriptorSets, {});
    pCommandBuffer->bindVertexBuffers(0, {_pFrameVerticesBuffer}, {0});
    pCommandBuffer->commandBuffer().draw(6, 1, 0, 0);
}
void RenderFrame::onUpdateUBO(uint32_t frameIndex) const {}

void RenderFrame::addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) {}

void RenderFrame::writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) {}