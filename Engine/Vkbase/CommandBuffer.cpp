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

CommandBuffer::CommandBuffer(const std::string &name, const VkResourceManagerHolder::WeakReference &pool, vk::CommandBuffer handle, bool oneTimeSubmit,
                             bool primary)
    : VkGpuResourceBase(VkResourceType::CommandBuffer, name, pool.lock<CommandPool>()->_device), _pool(connectTo(pool)), _commandBuffer(handle),
      _oneTimeSubmit(oneTimeSubmit), _primary(primary)
{
    if (auto p = _device.lock<Device>())
        _fence = p->device().createFence(vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled});
}

void CommandBuffer::cleanCounter()
{
    for (uint32_t *pCounter : _pCounters)
        --(*pCounter);
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

    vk::Device device;
    if (auto p = _device.lock<Device>())
        device = p->device();
    auto fence = _fence;
    auto commandBuffer = _commandBuffer;
    vk::CommandPool commandPool;
    if (auto p = _pool.lock<CommandPool>())
        commandPool = p->_commandPool;

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
    inherit.subpass = subpassIndex;
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue;
    if (auto p = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::RenderPass, renderPassName).lock<RenderPass>())
    {
        inherit.renderPass = p->renderPass();
        beginInfo.pInheritanceInfo = &inherit;
        _commandBuffer.begin(beginInfo);
        insertCounters(p->counters());
    }
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

    if (auto p = _device.lock<Device>())
        p->device().resetFences(_fence);

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(_commandBuffer).setWaitSemaphores(waitSemaphores).setWaitDstStageMask(waitDstStageMask).setSignalSemaphores(signalSemaphores);
    if (auto p = _pool.lock<CommandPool>())
        p->_queue.submit(submitInfo, _fence);
}

void CommandBuffer::reset()
{
    if (_oneTimeSubmit)
        throw std::runtime_error("[ERROR] Cannot reset a one-time CommandBuffer.");
    _pipeline = {};
    _indiceBuffer = {};
    _commandBuffer.reset({});
    _inRecording = false;
    cleanCounter();
}

const VkResourceManagerHolder::WeakReference &CommandBuffer::device() const { return _device; }

void CommandBuffer::bindPipeline(const VkResourceManagerHolder::WeakReference &pipeline)
{
    if (!_inRecording || _pipeline == pipeline || !pipeline.lock<Pipeline>())
        return;

    if (auto p = pipeline.lock<Pipeline>())
    {
        if (auto p1 = _device.lock())
            if (auto p2 = p->device().lock())
                if (p2->name() != p1->name())
                    throw std::runtime_error("Pipeline device mismatch with command buffer device");

        _commandBuffer.bindPipeline(p->pipelineBindPoint(), p->pipeline());
        insertCounters(p->counters());
    }
    _pipeline = pipeline;
}

void CommandBuffer::bindDescriptorSets(uint32_t firstSet,
                                       const std::vector<std::pair<VkResourceManagerHolder::WeakReference, std::pair<std::string, uint32_t>>> &pDescriptorSets,
                                       const vk::ArrayProxy<const uint32_t> &dynamicOffsets)
{
    if (!_pipeline.lock())
        throw std::runtime_error("You should bind pipeline first.");

    std::vector<vk::DescriptorSet> descriptorSets;
    descriptorSets.reserve(pDescriptorSets.size());

    for (const auto &[DescriptorSet, indexPair] : pDescriptorSets) // Using structured bindings
    {
        const auto &[setName, setIndex] = indexPair;
        if (auto p = DescriptorSet.lock<DescriptorSets>())
        {
            descriptorSets.push_back(p->sets(setName)[setIndex]);
            insertCounters(p->counters());
        }
    }

    if (auto p = _pipeline.lock<Pipeline>())
        _commandBuffer.bindDescriptorSets(p->pipelineBindPoint(), p->layout(), firstSet, descriptorSets, dynamicOffsets);
}

void CommandBuffer::bindVertexBuffers(uint32_t firstBinding, const vk::ArrayProxy<VkResourceManagerHolder::WeakReference> &buffers,
                                      const vk::ArrayProxy<const vk::DeviceSize> &offsets)
{
    std::vector<vk::Buffer> vkBuffers;
    vkBuffers.reserve(buffers.size());

    for (auto buffer : buffers)
    {
        if (auto p = buffer.lock<Buffer>())
        {
            vkBuffers.push_back(p->buffer());
            insertCounters(p->counters());
        }
        else
            throw std::runtime_error("Null buffer detected in bindVertexBuffers");
    }

    _commandBuffer.bindVertexBuffers(firstBinding, vkBuffers, offsets);
}

void CommandBuffer::bindIndexBuffer(const VkResourceManagerHolder::WeakReference &buffer, vk::DeviceSize offset, vk::IndexType indexType)
{
    if (_indiceBuffer == buffer)
        return;
    if (auto p = buffer.lock<Buffer>())
    {
        _commandBuffer.bindIndexBuffer(p->buffer(), offset, indexType);
        insertCounters(p->counters());
    }
    _indiceBuffer = buffer;
}

void CommandBuffer::beginRenderPass(const VkResourceManagerHolder::WeakReference &renderPass, const VkResourceManagerHolder::WeakReference &framebuffer,
                                    vk::RenderPassBeginInfo info, vk::SubpassContents subpassContents)
{
    if (auto p = renderPass.lock<VkGpuResourceBase>())
        insertCounters(p->counters());
    if (auto p = framebuffer.lock<VkGpuResourceBase>())
        insertCounters(p->counters());

    if (auto p = renderPass.lock<RenderPass>())
        if (auto p1 = framebuffer.lock<Framebuffer>())
            info.setFramebuffer(p1->framebuffer()).setRenderPass(p->renderPass());

    _commandBuffer.beginRenderPass(info, subpassContents);
}

void CommandBuffer::executeCommands(const std::vector<VkResourceManagerHolder::WeakReference> &commandBuffers)
{
    if (commandBuffers.empty())
        return;

    std::vector<vk::CommandBuffer> vkCommandBuffers;
    vkCommandBuffers.reserve(commandBuffers.size());

    for (const auto &commandBuffer : commandBuffers)
    {
        if (!commandBuffer.lock() || commandBuffer == weakReference())
            continue;

        if (auto p = _device.lock())
            if (auto p1 = commandBuffer.lock<CommandBuffer>())
                if (auto p2 = p1->device().lock())
                    if (p2->name() != p->name())
                        throw std::runtime_error("CommandBuffer device mismatch");

        if (auto p = commandBuffer.lock<CommandBuffer>())
        {
            vkCommandBuffers.push_back(p->commandBuffer());
            insertCounters(p->_pCounters);
        }
    }

    if (!vkCommandBuffers.empty())
        _commandBuffer.executeCommands(vkCommandBuffers);
}

void CommandBuffer::waitForFence()
{
    if (auto p = _device.lock<Device>())
        (void)p->device().waitForFences(_fence, vk::True, UINT64_MAX);
}

const std::string &CommandBuffer::commandPoolName() const
{
    if (auto p = _pool.lock())
        return p->name();
    throw std::runtime_error("CommandPool already destroyed.");
}

} // namespace Vkbase
