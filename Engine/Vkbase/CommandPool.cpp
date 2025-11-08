#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Device.h"

namespace Vkbase
{
CommandPool::CommandPool(const std::string &resourceName, const std::string &deviceName, CommandPoolQueueType queueType)
    : VkGpuResourceBase(VkResourceType::CommandPool, resourceName, resourceManager().resource(VkResourceType::Device, deviceName)),
      _queueIndex(determineQueueIndex(queueType)), _queue(determineQueue(queueType))
{
    determineQueue(queueType);
    createCommandPool();
}

CommandPool::~CommandPool()
{
    vk::Device device;
    if (auto p = _device.lock<Device>())
        device = p->device();
    auto commandPool = _commandPool;
    _onDelayDestroy = [device, commandPool]() { device.destroyCommandPool(commandPool); };
}

const vk::Queue &CommandPool::determineQueue(CommandPoolQueueType queueType) const
{
    if (auto p = _device.lock<Device>())
    {
        switch (queueType)
        {
        case CommandPoolQueueType::Graphics:
            return p->graphicsQueue();
        case CommandPoolQueueType::Compute:
            return p->computeQueue();
        case CommandPoolQueueType::Present:
            return p->presentQueue();
        }
    }
    throw std::runtime_error("[ERROR] Unknown queue type.");
}

uint32_t CommandPool::determineQueueIndex(CommandPoolQueueType queueType) const
{
    if (auto p = _device.lock<Device>())
    {
        switch (queueType)
        {
        case CommandPoolQueueType::Graphics:
            return p->queueFamilyIndices().graphicsFamilyIndex;
        case CommandPoolQueueType::Compute:
            return p->queueFamilyIndices().computeFamilyIndex;
        case CommandPoolQueueType::Present:
            return p->queueFamilyIndices().presentFamilyIndex;
        }
    }
    throw std::runtime_error("[ERROR] Unknown queue type.");
}

void CommandPool::createCommandPool()
{
    vk::CommandPoolCreateInfo createInfo;
    createInfo.setQueueFamilyIndex(_queueIndex).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    if (auto p = _device.lock<Device>())
        _commandPool = p->device().createCommandPool(createInfo);
}

std::vector<VkResourceManagerHolder::WeakReference> CommandPool::allocateFlightCommandBuffers(uint32_t count) const
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setCommandPool(_commandPool).setCommandBufferCount(count).setLevel(vk::CommandBufferLevel::ePrimary);

    std::vector<vk::CommandBuffer> commandBuffers;
    if (auto p = _device.lock<Device>())
        commandBuffers = p->device().allocateCommandBuffers(allocateInfo);
    std::vector<VkResourceManagerHolder::WeakReference> pCommandBuffers;
    pCommandBuffers.reserve(commandBuffers.size());
    auto thisReference = weakReference();
    for (const vk::CommandBuffer &commandBuffer : commandBuffers)
        pCommandBuffers.push_back(createResource<CommandBuffer>(std::string(""), thisReference, commandBuffer, false, true));

    return pCommandBuffers;
}

std::vector<VkResourceManagerHolder::WeakReference> CommandPool::allocateSecondaryCommandBuffers(uint32_t count) const
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setCommandPool(_commandPool).setCommandBufferCount(count).setLevel(vk::CommandBufferLevel::eSecondary);

    std::vector<vk::CommandBuffer> commandBuffers;
    if (auto p = _device.lock<Device>())
        commandBuffers = p->device().allocateCommandBuffers(allocateInfo);
    std::vector<VkResourceManagerHolder::WeakReference> pCommandBuffers;
    pCommandBuffers.reserve(commandBuffers.size());
    auto thisReference = weakReference();
    for (const vk::CommandBuffer &commandBuffer : commandBuffers)
        pCommandBuffers.push_back(createResource<CommandBuffer>(std::string(""), thisReference, commandBuffer, false, false));

    return pCommandBuffers;
}

VkResourceManagerHolder::WeakReference CommandPool::allocateOnceCommandBuffer() const
{
    VkResourceManagerHolder::WeakReference commandBuffer;
    auto thisReference = weakReference();
    if (auto p = _device.lock<Device>())
        commandBuffer = createResource<CommandBuffer>(
            std::string(""), thisReference,
            p->device().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo().setCommandPool(_commandPool).setCommandBufferCount(1).setLevel(vk::CommandBufferLevel::ePrimary))[0],
            true, true);

    if (auto p = commandBuffer.lock<CommandBuffer>())
        p->begin();
    return commandBuffer;
}

void CommandPool::endOnceCommandBuffer(const VkResourceManagerHolder::WeakReference &commandBuffer) const
{
    if (auto p = commandBuffer.lock<CommandBuffer>())
    {
        p->end();
        p->submit();
    }
    freeCommandBuffers(commandBuffer);
}

void CommandPool::freeCommandBuffers(const vk::ArrayProxy<VkResourceManagerHolder::WeakReference> &commandBuffers) const
{
    for (VkResourceManagerHolder::WeakReference commandBuffer : commandBuffers)
        if (auto p = commandBuffer.lock())
            p->destroy();
}

VkResourceManagerHolder::WeakReference CommandPool::getCommandPool(const std::string &deviceName, CommandPoolQueueType queueType)
{
    VkResourceManagerHolder::WeakReference commandPool = resourceManager().resource(Vkbase::VkResourceType::CommandPool, toString(queueType) + deviceName);

    if (commandPool.lock())
        return commandPool;
    else
        return createResource<CommandPool>(toString(queueType) + deviceName, deviceName, queueType);
}
}; // namespace Vkbase