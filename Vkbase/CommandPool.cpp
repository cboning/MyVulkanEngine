#include "CommandPool.h"
#include "Device.h"

namespace Vkbase
{
    CommandPool::CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType)
        : ResourceBase(ResourceType::CommandPool, resourceName), _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(ResourceType::Device, deviceName)))), _queue(determineQueue(queueType)), _queueIndex(determineQueueIndex(queueType))
    {
        determineQueue(queueType);
        createCommandPool();
    }

    CommandPool::~CommandPool()
    {
        _device.device().destroyCommandPool(_commandPool);
    }

    const vk::Queue &CommandPool::determineQueue(CommandPoolQueueType queueType) const
    {
        switch (queueType)
        {
            case CommandPoolQueueType::Graphics:
                return _device.graphicsQueue();
            case CommandPoolQueueType::Compute:
                return _device.computeQueue();
            case CommandPoolQueueType::Present:
                return _device.presentQueue();
        }
        throw std::runtime_error("[ERROR] Unknown queue type.");
    }

    const uint32_t CommandPool::determineQueueIndex(CommandPoolQueueType queueType) const
    {
        switch (queueType)
        {
            case CommandPoolQueueType::Graphics:
                return _device.queueFamilyIndices().graphicsFamilyIndex;
            case CommandPoolQueueType::Compute:
                return _device.queueFamilyIndices().computeFamilyIndex;
            case CommandPoolQueueType::Present:
                return _device.queueFamilyIndices().presentFamilyIndex;
        }
        throw std::runtime_error("[ERROR] Unknown queue type.");
    }
    
    void CommandPool::createCommandPool()
    {
        vk::CommandPoolCreateInfo createInfo;
        createInfo.setQueueFamilyIndex(_queueIndex)
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        _commandPool = _device.device().createCommandPool(createInfo);
    }

    std::vector<vk::CommandBuffer> CommandPool::allocateFlightCommandBuffers(uint32_t maxFlightFrameCount) const
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandPool(_commandPool)
            .setCommandBufferCount(maxFlightFrameCount)
            .setLevel(vk::CommandBufferLevel::ePrimary);
        
        return _device.device().allocateCommandBuffers(allocateInfo);
    }

    vk::CommandBuffer CommandPool::allocateOnceCommandBuffer() const
    {
        vk::CommandBuffer commandBuffer = _device.device().allocateCommandBuffers(vk::CommandBufferAllocateInfo()
            .setCommandPool(_commandPool)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary)
        )[0];

        commandBuffer.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        return commandBuffer;
    }

    void CommandPool::endOnceCommandBuffer(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer);
        _queue.submit(submitInfo);
        _queue.waitIdle();
        freeCommandBuffers(commandBuffer);
    }

    void CommandPool::freeCommandBuffers(const vk::ArrayProxy<const vk::CommandBuffer> &commandBuffers) const
    {
        _device.device().freeCommandBuffers(_commandPool, commandBuffers);
    }

    const CommandPool &CommandPool::getCommandPool(const std::string &deviceName, CommandPoolQueueType queueType)
    {
        const CommandPool *pCommandPool = dynamic_cast<const CommandPool *>(resourceManager().resource(Vkbase::ResourceType::CommandPool, toString(queueType) + deviceName));
        if (pCommandPool)
            return *pCommandPool;
        else
            return *(new CommandPool(toString(queueType) + deviceName, deviceName, queueType));
    }
};