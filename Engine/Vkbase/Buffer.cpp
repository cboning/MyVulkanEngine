#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"

namespace Vkbase
{
Buffer::Buffer(const std::string &resourceName, const std::string &deviceName, vk::DeviceSize size, vk::BufferUsageFlags usage, void *pData)
    : VkGpuResourceBase(Vkbase::VkResourceType::Buffer, resourceName, resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)), _size(size)
{
    if (pData)
    {
        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingMemory;
        if (auto p = _device.lock<Device>())
        {
            createBuffer(vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                         stagingBuffer, stagingMemory);
            createBuffer(vk::BufferUsageFlagBits::eTransferDst | usage, vk::MemoryPropertyFlagBits::eDeviceLocal, _buffer, _memory);
            memcpy(p->device().mapMemory(stagingMemory, 0, _size), pData, _size);
            p->device().unmapMemory(stagingMemory);

            copyBuffer(stagingBuffer, _buffer);

            p->device().freeMemory(stagingMemory);
            p->device().destroy(stagingBuffer);
        }
    }
    else
    {
        createBuffer(usage, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, _buffer, _memory);
    }
}

Buffer::~Buffer()
{
    if (auto p = _device.lock<Device>())
    {
        auto device = p->device();
        auto memory = _memory;
        auto buffer = _buffer;

        _onDelayDestroy = [device, memory, buffer]()
        {
            device.freeMemory(memory);
            device.destroy(buffer);
        };
    }
}

void Buffer::createBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties, vk::Buffer &buffer, vk::DeviceMemory &memory)
{
    vk::BufferCreateInfo createInfo;
    createInfo.setSize(_size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    if (auto p = _device.lock<Device>())
    {
        buffer = p->device().createBuffer(createInfo);
        vk::MemoryRequirements requirements = p->device().getBufferMemoryRequirements(buffer);

        vk::MemoryAllocateInfo allocateInfo;
        allocateInfo.setAllocationSize(requirements.size).setMemoryTypeIndex(findMemoryTypeIndex(requirements.memoryTypeBits, memoryProperties));

        memory = p->device().allocateMemory(allocateInfo);

        p->device().bindBufferMemory(buffer, memory, 0);
    }
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
        memcpy(p->device().mapMemory(_memory, 0, _size), pData, _size);
        p->device().unmapMemory(_memory);
    }
}

const vk::Buffer &Buffer::buffer() const { return _buffer; }

vk::DeviceSize Buffer::size() const { return _size; }
} // namespace Vkbase