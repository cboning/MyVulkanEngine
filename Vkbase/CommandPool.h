#pragma once
#include "VkResourceBase.h"

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
class CommandPool : public VkResourceBase
{
    friend class VkResourceManager;
    friend class CommandBuffer;

private:
    vk::CommandPool _commandPool;
    const Device &_device;
    const uint32_t _queueIndex;
    const vk::Queue &_queue;

    void createCommandPool();
    const vk::Queue &determineQueue(CommandPoolQueueType queueType) const;
    uint32_t determineQueueIndex(CommandPoolQueueType queueType) const;
    CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType);
    ~CommandPool();

public:
    std::vector<CommandBuffer *> allocateFlightCommandBuffers(uint32_t maxFlightFrameCount) const;
    std::vector<CommandBuffer *> allocateSecondaryCommandBuffers(uint32_t count) const;
    CommandBuffer *allocateOnceCommandBuffer() const;
    void endOnceCommandBuffer(CommandBuffer *pCommandBuffer) const;
    void freeCommandBuffers(const vk::ArrayProxy<CommandBuffer *> &pCommandBuffers) const;
    static CommandPool &getCommandPool(const std::string &deviceName, CommandPoolQueueType queueType);
};
} // namespace Vkbase
