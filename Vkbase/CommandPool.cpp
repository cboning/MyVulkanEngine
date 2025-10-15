#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Device.h"

namespace Vkbase
{
    CommandPool::CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType)
        : ResourceBase(ResourceType::CommandPool, resourceName), _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(ResourceType::Device, deviceName)))), _queueIndex(determineQueueIndex(queueType)), _queue(determineQueue(queueType))
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

    uint32_t CommandPool::determineQueueIndex(CommandPoolQueueType queueType) const
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

    std::vector<CommandBuffer *> CommandPool::allocateFlightCommandBuffers(uint32_t maxFlightFrameCount) const
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandPool(_commandPool)
            .setCommandBufferCount(maxFlightFrameCount)
            .setLevel(vk::CommandBufferLevel::ePrimary);
        
        auto commandBuffers = _device.device().allocateCommandBuffers(allocateInfo);
        std::vector<CommandBuffer *> pCommandBuffers;
        pCommandBuffers.reserve(commandBuffers.size());
        for (const vk::CommandBuffer &commandBuffer : commandBuffers)
            pCommandBuffers.push_back(createResource<CommandBuffer>(std::string(""), *this, commandBuffer, false));
        
        return pCommandBuffers;
    }

    CommandBuffer *CommandPool::allocateOnceCommandBuffer() const
    {
        CommandBuffer *pCommandBuffer = createResource<CommandBuffer>(std::string(""), *this, _device.device().allocateCommandBuffers(vk::CommandBufferAllocateInfo()
            .setCommandPool(_commandPool)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary)
        )[0], true);
        pCommandBuffer->begin();
        return pCommandBuffer;
    }

    void CommandPool::endOnceCommandBuffer(CommandBuffer *pCommandBuffer) const
    {
        pCommandBuffer->end();
        pCommandBuffer->submit();
        freeCommandBuffers(pCommandBuffer);
    }

    void CommandPool::freeCommandBuffers(const vk::ArrayProxy<CommandBuffer *> &pCommandBuffers) const
    {
        for (const CommandBuffer *pCommandBuffer : pCommandBuffers)
            pCommandBuffer->destroy();
    }

    CommandPool &CommandPool::getCommandPool(const std::string &deviceName, CommandPoolQueueType queueType)
    {
        CommandPool *pCommandPool = dynamic_cast<CommandPool *>(resourceManager().resource(Vkbase::ResourceType::CommandPool, toString(queueType) + deviceName));
        if (pCommandPool)
            return *pCommandPool;
        else
            return *(new CommandPool(toString(queueType) + deviceName, deviceName, queueType));
    }
};