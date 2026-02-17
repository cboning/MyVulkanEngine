#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"
#include "MemoryAllocator.h"

namespace Vkbase
{
Buffer::Buffer(const std::string &resourceName, const std::string &deviceName, vk::DeviceSize size, vk::BufferUsageFlags usage, void *pData)
    : VkGpuResourceBase(Vkbase::VkResourceType::Buffer, resourceName, resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)), _size(size)
{
    if (pData)
    {
        vk::Buffer stagingBuffer;
        if (auto p = _device.lock<Device>())
        {
            MemoryAllocator::Allocation allocation = createBuffer(
                vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);
            _memory = createBuffer(vk::BufferUsageFlagBits::eTransferDst | usage, vk::MemoryPropertyFlagBits::eDeviceLocal, _buffer);

            memcpy(p->memoryAllocator().map(allocation), pData, _size);
            p->memoryAllocator().unmap(allocation);

            copyBuffer(stagingBuffer, _buffer);

            p->memoryAllocator().free(allocation);
            p->device().destroy(stagingBuffer);
        }
    }
    else
    {
        _memory = createBuffer(usage, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, _buffer);
    }
}

Buffer::~Buffer()
{
    if (auto p = _device.lock<Device>())
    {
        auto device = p;
        auto memory = _memory;
        auto buffer = _buffer;

        _onDelayDestroy = [device, memory, buffer]()
        {
            device->memoryAllocator().free(memory);
            device->device().destroy(buffer);
        };
    }
}

MemoryAllocator::Allocation Buffer::createBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties, vk::Buffer &buffer)
{
    vk::BufferCreateInfo createInfo;
    createInfo.setSize(_size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    if (auto p = _device.lock<Device>())
    {
        buffer = p->device().createBuffer(createInfo);
        vk::MemoryRequirements requirements = p->device().getBufferMemoryRequirements(buffer);

        std::optional<MemoryAllocator::Allocation> allocation =
            p->memoryAllocator().allocate(requirements.size, 256, requirements.memoryTypeBits, memoryProperties);
        if (!allocation.has_value())
            throw std::runtime_error("Failed to allocate the memory.");
        _memory = allocation.value();
        p->device().bindBufferMemory(buffer, _memory.memory(), _memory.offset());
        return allocation.value();
    }

    throw std::runtime_error("Failed to found Device or the device already destroyed.");
}

uint32_t Buffer::findMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    if (auto p = _device.lock<Device>())
    {
        vk::PhysicalDeviceMemoryProperties memoryProperties = p->physicalDevice().getMemoryProperties();
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            if (typeFilter & (1 << i) && (properties & memoryProperties.memoryTypes[i].propertyFlags) == properties)
                return i;
        }
    }
    throw std::runtime_error("[ERROR] Failed to find suitable memory type!");
}

void Buffer::copyBuffer(vk::Buffer &src, vk::Buffer &dst)
{
    vk::BufferCopy region;
    region.setSrcOffset(0).setDstOffset(0).setSize(_size);

    if (auto p = _device.lock())
        if (auto p1 = CommandPool::getCommandPool(p->name(), Vkbase::CommandPoolQueueType::Graphics).lock<CommandPool>())
        {
            auto commandBuffer = p1->allocateOnceCommandBuffer();
            if (auto p = commandBuffer.lock<CommandBuffer>())
                p->commandBuffer().copyBuffer(src, dst, region);

            p1->endOnceCommandBuffer(commandBuffer);
        }
}

void Buffer::updateBufferData(const void *pData) const
{
    if (auto p = _device.lock<Device>())
    {
        memcpy(p->memoryAllocator().map(_memory), pData, _size);
        p->memoryAllocator().unmap(_memory);
    }
}

const vk::Buffer &Buffer::buffer() const { return _buffer; }

vk::DeviceSize Buffer::size() const { return _size; }
} // namespace Vkbase