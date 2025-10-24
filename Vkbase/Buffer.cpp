#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"

namespace Vkbase
{
Buffer::Buffer(const std::string &resourceName, const std::string &deviceName, vk::DeviceSize size, vk::BufferUsageFlags usage, void *pData)
    : VkGpuResourceBase(Vkbase::VkResourceType::Buffer, resourceName,
                        *dynamic_cast<Device *>(resourceManager().resource(Vkbase::VkResourceType::Device, deviceName))),
      _size(size)
{
    if (pData)
    {
        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingMemory;
        createBuffer(vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
                     stagingMemory);
        createBuffer(vk::BufferUsageFlagBits::eTransferDst | usage, vk::MemoryPropertyFlagBits::eDeviceLocal, _buffer, _memory);
        memcpy(_device.device().mapMemory(stagingMemory, 0, _size), pData, _size);
        _device.device().unmapMemory(stagingMemory);

        copyBuffer(stagingBuffer, _buffer);

        _device.device().freeMemory(stagingMemory);
        _device.device().destroy(stagingBuffer);
    }
    else
    {
        createBuffer(usage, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, _buffer, _memory);
    }
}

Buffer::~Buffer()
{
    auto device = _device.device();
    auto memory = _memory;
    auto buffer = _buffer;

    _onDelayDestroy = [device, memory, buffer]()
    {
        device.freeMemory(memory);
        device.destroy(buffer);
    };
}

void Buffer::createBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties, vk::Buffer &buffer, vk::DeviceMemory &memory)
{
    vk::BufferCreateInfo createInfo;
    createInfo.setSize(_size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    buffer = _device.device().createBuffer(createInfo);
    vk::MemoryRequirements requirements = _device.device().getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.setAllocationSize(requirements.size).setMemoryTypeIndex(findMemoryTypeIndex(requirements.memoryTypeBits, memoryProperties));

    memory = _device.device().allocateMemory(allocateInfo);

    _device.device().bindBufferMemory(buffer, memory, 0);
}

uint32_t Buffer::findMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = _device.physicalDevice().getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i) && (properties & memoryProperties.memoryTypes[i].propertyFlags) == properties)
            return i;
    }
    throw std::runtime_error("[ERROR] Failed to find suitable memory type!");
}

void Buffer::copyBuffer(vk::Buffer &src, vk::Buffer &dst)
{
    vk::BufferCopy region;
    region.setSrcOffset(0).setDstOffset(0).setSize(_size);

    CommandPool &commandPool = CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Graphics);
    CommandBuffer *pCommandBuffer = commandPool.allocateOnceCommandBuffer();

    pCommandBuffer->commandBuffer().copyBuffer(src, dst, region);

    commandPool.endOnceCommandBuffer(pCommandBuffer);
}

void Buffer::updateBufferData(const void *pData) const
{
    memcpy(_device.device().mapMemory(_memory, 0, _size), pData, _size);
    _device.device().unmapMemory(_memory);
}

const vk::Buffer &Buffer::buffer() const { return _buffer; }

vk::DeviceSize Buffer::size() const { return _size; }
} // namespace Vkbase