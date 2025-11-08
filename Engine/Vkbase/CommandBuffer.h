#pragma once
#include "CommandPool.h"
#include "VkGpuResourceBase.h"
#include "VkGpuResourceGarbageCollector.h"
#include <unordered_set>

namespace Vkbase
{
class Pipeline;
class DescriptorSets;
class Buffer;
class RenderPass;
class Framebuffer;
class CommandBuffer : public VkGpuResourceBase
{
    friend class VkResourceManager;

private:
    VkResourceManagerHolder::WeakReference _pool;
    vk::CommandBuffer _commandBuffer;
    std::unordered_set<uint32_t *> _pCounters;
    const bool _oneTimeSubmit = false;
    vk::Fence _fence;
    bool _inRecording = false;
    VkResourceManagerHolder::WeakReference _pipeline;
    VkResourceManagerHolder::WeakReference _indiceBuffer;
    bool _primary;

    CommandBuffer(const std::string &name, const VkResourceManagerHolder::WeakReference &pool, vk::CommandBuffer handle, bool oneTimeSubmit, bool primary);

    void cleanCounter();
    void insertCounters(const std::unordered_set<uint32_t *> &pCounters);

public:
    ~CommandBuffer();

    void begin();
    void begin(const std::string &renderPassName, uint32_t subpassIndex);
    void end();
    void submit(const std::vector<vk::Semaphore> &waitSemaphores = {}, const std::vector<vk::PipelineStageFlags> &waitDstStageMask = {},
                const std::vector<vk::Semaphore> &signalSemaphores = {});

    void reset();

    vk::CommandBuffer commandBuffer() const { return _commandBuffer; }
    const VkResourceManagerHolder::WeakReference &device() const;
    void bindPipeline(const VkResourceManagerHolder::WeakReference &pPipeline);
    void bindDescriptorSets(uint32_t firstSet, const std::vector<std::pair<VkResourceManagerHolder::WeakReference, std::pair<std::string, uint32_t>>> &pDescriptorSets,
                            const vk::ArrayProxy<const uint32_t> &dynamicOffsets);
    void bindVertexBuffers(uint32_t firstBinding, const vk::ArrayProxy<VkResourceManagerHolder::WeakReference> &buffers,
                           const vk::ArrayProxy<const vk::DeviceSize> &offsets);

    void bindIndexBuffer(const VkResourceManagerHolder::WeakReference &buffer, vk::DeviceSize offset, vk::IndexType indexType);
    void beginRenderPass(const VkResourceManagerHolder::WeakReference &renderPass, const VkResourceManagerHolder::WeakReference &framebuffer, vk::RenderPassBeginInfo info,
                         vk::SubpassContents subpassContents);
    void executeCommands(const std::vector<VkResourceManagerHolder::WeakReference> &pCommandBuffers);

    void waitForFence();
    const std::string &commandPoolName() const;
};
} // namespace Vkbase
