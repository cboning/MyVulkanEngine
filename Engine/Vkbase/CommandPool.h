#pragma once
#include "VkGpuResourceBase.h"

namespace Vkbase
{
enum class CommandPoolQueueType
{
    Graphics,
    Compute,
    Present
};
inline const std::string toString(CommandPoolQueueType type)
{
    switch (type)
    {
    case CommandPoolQueueType::Graphics:
        return "Graphics";
    case CommandPoolQueueType::Compute:
        return "Compute";
    case CommandPoolQueueType::Present:
        return "Present";
    }
}
class Device;
class CommandBuffer;
class CommandPool : public VkGpuResourceBase
{
    friend class VkResourceManager;
    friend class CommandBuffer;

private:
    vk::CommandPool _commandPool;
    const uint32_t _queueIndex;
    const vk::Queue &_queue;

    void createCommandPool();
    const vk::Queue &determineQueue(CommandPoolQueueType queueType) const;
    uint32_t determineQueueIndex(CommandPoolQueueType queueType) const;
    CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType);
    ~CommandPool();

public:
    std::vector<VkResourceManagerHolder::WeakReference> allocateFlightCommandBuffers(uint32_t maxFlightFrameCount) const;
    std::vector<VkResourceManagerHolder::WeakReference> allocateSecondaryCommandBuffers(uint32_t count) const;
    VkResourceManagerHolder::WeakReference allocateOnceCommandBuffer() const;
    void endOnceCommandBuffer(const VkResourceManagerHolder::WeakReference &commandBuffer) const;
    void freeCommandBuffers(const vk::ArrayProxy<VkResourceManagerHolder::WeakReference> &commandBuffers) const;
    static VkResourceManagerHolder::WeakReference getCommandPool(const std::string &deviceName, CommandPoolQueueType queueType);
};
} // namespace Vkbase
