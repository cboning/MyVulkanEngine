#include "CommandBuffer.h"
#include "Buffer.h"
#include "CommandPool.h"
#include "DescriptorSets.h"
#include "Device.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "VkGpuResourceGarbageCollector.h"

namespace Vkbase
{

CommandBuffer::CommandBuffer(const std::string &name, const CommandPool &pool, vk::CommandBuffer handle, bool oneTimeSubmit, bool primary)
    : VkGpuResourceBase(VkResourceType::CommandBuffer, name, pool._device), _pool(*connectTo(&pool)), _commandBuffer(handle),
      _oneTimeSubmit(oneTimeSubmit), _fence(_device.device().createFence(vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled})), _primary(primary)
{
}

void CommandBuffer::cleanCounter()
{
    for (uint32_t *pCounter : _pCounters)
        (*pCounter)--;
    _pCounters.clear();
}

void CommandBuffer::insertCounters(const std::unordered_set<uint32_t *> &pCounters)
{
    for (uint32_t *pCounter : pCounters)
    {
        auto result = _pCounters.insert(pCounter);
        if (result.second)
            ++*pCounter;
    }
}

CommandBuffer::~CommandBuffer()
{
    waitForFence();
    cleanCounter();

    auto device = _device.device();
    auto fence = _fence;
    auto commandBuffer = _commandBuffer;
    auto commandPool = _pool._commandPool;

    _onDelayDestroy = [device, fence, commandBuffer, commandPool]()
    {
        if (commandBuffer)
            device.freeCommandBuffers(commandPool, commandBuffer);
        device.destroy(fence);
    };
}

void CommandBuffer::begin()
{
    if (!_primary)
        throw std::runtime_error("[ERROR] This method is given for primary CommandBuffer, but it is a secondary CommandBuffer.");

    if (_inRecording)
        throw std::runtime_error("[ERROR] CommandBuffer already in recording state.");

    vk::CommandBufferBeginInfo beginInfo;
    if (_oneTimeSubmit)
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    _commandBuffer.begin(beginInfo);
    _inRecording = true;
}

void CommandBuffer::begin(const std::string &renderPassName, uint32_t subpassIndex)
{
    if (_primary)
        throw std::runtime_error("[ERROR] This method is given for secondary CommandBuffer, but it is a primary CommandBuffer.");

    if (_inRecording)
        throw std::runtime_error("[ERROR] CommandBuffer already in recording state.");

    vk::CommandBufferInheritanceInfo inherit = {};
    Vkbase::RenderPass *pRenderPass =
        dynamic_cast<Vkbase::RenderPass *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::RenderPass, renderPassName));

    inherit.renderPass = pRenderPass->renderPass();
    inherit.subpass = subpassIndex;

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue;
    beginInfo.pInheritanceInfo = &inherit;

    _commandBuffer.begin(beginInfo);
    insertCounters(pRenderPass->counters());
    _inRecording = true;
}

void CommandBuffer::end()
{
    if (!_inRecording)
        throw std::runtime_error("[ERROR] CommandBuffer::end() called without begin().");

    _commandBuffer.end();
    _inRecording = false;
}

void CommandBuffer::submit(const std::vector<vk::Semaphore> &waitSemaphores, const std::vector<vk::PipelineStageFlags> &waitDstStageMask,
                           const std::vector<vk::Semaphore> &signalSemaphores)
{
    if (_inRecording)
        throw std::runtime_error("[ERROR] Cannot submit CommandBuffer while still recording.");
    _device.device().resetFences(_fence);

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(_commandBuffer).setWaitSemaphores(waitSemaphores).setWaitDstStageMask(waitDstStageMask).setSignalSemaphores(signalSemaphores);
    _pool._queue.submit(submitInfo, _fence);
}

void CommandBuffer::reset()
{
    if (_oneTimeSubmit)
        throw std::runtime_error("[ERROR] Cannot reset a one-time CommandBuffer.");
    _pPipeline = nullptr;
    _pIndiceBuffer = nullptr;
    _commandBuffer.reset({});
    _inRecording = false;
    cleanCounter();
}

const Device &CommandBuffer::device() { return _device; }

void CommandBuffer::bindPipeline(Pipeline *pPipeline)
{
    if (!_inRecording)
        return;

    if (_pPipeline == pPipeline)
        return;

    if (pPipeline->device().name() != _device.name())
        throw std::runtime_error("");

    _commandBuffer.bindPipeline(pPipeline->pipelineBindPoint(), pPipeline->pipeline());
    _pPipeline = pPipeline;
    insertCounters(pPipeline->counters());
}

void CommandBuffer::bindDescriptorSets(uint32_t firstSet, const std::vector<std::pair<DescriptorSets *, std::pair<std::string, uint32_t>>> &pDescriptorSets,
                                       const vk::ArrayProxy<const uint32_t> &dynamicOffsets)
{
    if (!_pPipeline)
        throw std::runtime_error("You should bind pipeline first.");

    std::vector<vk::DescriptorSet> descriptorSets;
    descriptorSets.reserve(pDescriptorSets.size());

    for (auto pDescriptorSet : pDescriptorSets)
    {
        descriptorSets.push_back(pDescriptorSet.first->sets(pDescriptorSet.second.first)[pDescriptorSet.second.second]);
        insertCounters(pDescriptorSet.first->counters());
    }

    _commandBuffer.bindDescriptorSets(_pPipeline->pipelineBindPoint(), _pPipeline->layout(), firstSet, descriptorSets, dynamicOffsets);
}

void CommandBuffer::bindVertexBuffers(uint32_t firstBinding, const vk::ArrayProxy<Buffer *> &buffers, const vk::ArrayProxy<const vk::DeviceSize> &offsets)
{
    std::vector<vk::Buffer> vkBuffers;
    for (auto *pBuffer : buffers)
        if (pBuffer == nullptr)
            throw std::runtime_error("Exist Empty Buffer.");

    vkBuffers.reserve(buffers.size());
    for (auto *pBuffer : buffers)
    {
        vkBuffers.push_back(pBuffer->buffer());
        insertCounters(pBuffer->counters());
    }

    _commandBuffer.bindVertexBuffers(firstBinding, vkBuffers, offsets);
}

void CommandBuffer::bindIndexBuffer(Buffer *pBuffer, vk::DeviceSize offset, vk::IndexType indexType)
{
    if (_pIndiceBuffer == pBuffer)
        return;
    _commandBuffer.bindIndexBuffer(pBuffer->buffer(), offset, indexType);
    _pIndiceBuffer = pBuffer;
    insertCounters(pBuffer->counters());
}

void CommandBuffer::beginRenderPass(RenderPass *pRenderPass, Framebuffer *pFramebuffer, vk::RenderPassBeginInfo info, vk::SubpassContents subpassContents)
{
    insertCounters(pRenderPass->counters());
    insertCounters(pFramebuffer->counters());

    info.setFramebuffer(pFramebuffer->framebuffer()).setRenderPass(pRenderPass->renderPass());

    _commandBuffer.beginRenderPass(info, subpassContents);
}

void CommandBuffer::executeCommands(const std::vector<CommandBuffer *> &pCommandBuffers)
{
    std::vector<vk::CommandBuffer> commandBuffers;
    commandBuffers.reserve(pCommandBuffers.size());
    for (const CommandBuffer *pCommandBuffer : pCommandBuffers)
    {
        commandBuffers.push_back(pCommandBuffer->commandBuffer());
        insertCounters(pCommandBuffer->_pCounters);
    }
    _commandBuffer.executeCommands(commandBuffers);
}

void CommandBuffer::waitForFence() { (void)_device.device().waitForFences(_fence, vk::True, UINT64_MAX); }

const std::string &CommandBuffer::commandPoolName() const { return _pool.name(); }

} // namespace Vkbase
