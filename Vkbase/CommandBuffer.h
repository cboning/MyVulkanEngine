#pragma once
#include "CommandPool.h"
#include "GpuResourceGarbageCollector.h"
#include "ResourceBase.h"
#include <unordered_set>

namespace Vkbase
{
class Pipeline;
class DescriptorSets;
class Buffer;
class RenderPass;
class Framebuffer;
class CommandBuffer : public ResourceBase
{
    friend class ResourceManager;

private:
    const Device &_device;
    const CommandPool &_pool;
    vk::CommandBuffer _commandBuffer;
    std::unordered_set<uint32_t *> _pCounters;
    vk::Fence _fence;
    const bool _oneTimeSubmit = false;
    bool _inRecording = false;
    Pipeline *_pPipeline = nullptr;

    CommandBuffer(const std::string &name, const CommandPool &pool, vk::CommandBuffer handle, bool oneTimeSubmit);

    void cleanCounter();
    void insertCounters(const std::unordered_set<uint32_t *> &pCounters);

public:
    ~CommandBuffer();

    void begin();
    void end();
    void submit(const std::vector<vk::Semaphore> &waitSemaphores = {}, const std::vector<vk::PipelineStageFlags> &waitDstStageMask = {},
                const std::vector<vk::Semaphore> &signalSemaphores = {});

    void reset();

    vk::CommandBuffer commandBuffer() const { return _commandBuffer; }
    void bindPipeline(Pipeline *pPipeline);
    void bindDescriptorSets(uint32_t firstSet, const std::vector<std::pair<DescriptorSets *, std::pair<std::string, uint32_t>>> &pDescriptorSets,
                            const vk::ArrayProxy<const uint32_t> &dynamicOffsets);
    void bindVertexBuffers(uint32_t firstBinding, const vk::ArrayProxy<Buffer *> &buffers, const vk::ArrayProxy<const vk::DeviceSize> &offsets);

    void bindIndexBuffer(Buffer *pBuffer, vk::DeviceSize offset, vk::IndexType indexType);
    void beginRenderPass(RenderPass *pRenderPass, Framebuffer *pFramebuffer, vk::RenderPassBeginInfo info, vk::SubpassContents subpassContents);

    void waitForFence();
};
} // namespace Vkbase
