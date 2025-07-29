#pragma once
#include "ResourceBase.h"

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
    class CommandPool : public ResourceBase
    {
    private:
        vk::CommandPool _commandPool;
        const Device &_device;
        const uint32_t _queueIndex;
        const vk::Queue &_queue;

        void createCommandPool();
        const vk::Queue &determineQueue(CommandPoolQueueType queueType) const;
        uint32_t determineQueueIndex(CommandPoolQueueType queueType) const;
        CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType);
        
    public:
        ~CommandPool();
        std::vector<vk::CommandBuffer> allocateFlightCommandBuffers(uint32_t maxFlightFrameCount) const;
        vk::CommandBuffer allocateOnceCommandBuffer() const;
        void endOnceCommandBuffer(vk::CommandBuffer commandBuffer) const;
        void freeCommandBuffers(const vk::ArrayProxy<const vk::CommandBuffer> &commandBuffers) const;
        static const CommandPool &getCommandPool(const std::string &deviceName, CommandPoolQueueType queueType);
    };
}
