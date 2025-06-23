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
    class CommandPool : public ResourceBase
    {
    private:
        vk::CommandPool _commandPool;
        Device &_device;
        uint32_t _queueIndex;
        vk::Queue &_queue;

        void createCommandPool();
        void determineQueue(CommandPoolQueueType queueType) const;
        
    public:
        CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType);
        ~CommandPool();
        std::vector<vk::CommandBuffer> allocateFlightCommandBuffers(uint32_t maxFlightFrameCount);
        vk::CommandBuffer allocateOnceCommandBuffer();
        void endOnceCommandBuffer(vk::CommandBuffer commandBuffer);
        void freeCommandBuffers(const vk::ArrayProxy<const vk::CommandBuffer> &commandBuffers);
    };
}
