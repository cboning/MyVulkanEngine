#pragma once
#include <vulkan/vulkan.hpp>
#include "ResourceBase.h"

namespace Vkbase
{
    enum class CommandPoolQueueType
    {
        Graphics,
        Compute,
        Present
    };
    class Device;
    class CommandBuffer;
    class CommandPool : public ResourceBase
    {
    private:
        vk::CommandPool _commandPool;
        const Device &_device;
        const uint32_t _queueIndex;
        const vk::Queue &_queue;

        void createCommandPool();
        const vk::Queue &determineQueue(CommandPoolQueueType queueType) const;
        const uint32_t determineQueueIndex(CommandPoolQueueType queueType) const;
        
    public:
        CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType);
        ~CommandPool();
        std::vector<vk::CommandBuffer> allocateFlightCommandBuffers(uint32_t maxFlightFrameCount);
        vk::CommandBuffer allocateOnceCommandBuffer();
        void endOnceCommandBuffer(vk::CommandBuffer commandBuffer);
        void freeCommandBuffers(const vk::ArrayProxy<const vk::CommandBuffer> &commandBuffers);
    };
}
