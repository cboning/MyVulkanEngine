#pragma once
#include "MemoryAllocator.h"
#include "VkGpuResourceBase.h"

namespace Vkbase
{
class Buffer : public VkGpuResourceBase
{
    friend class VkResourceManager;
    friend class DescriptorSets;
    friend class Image;
    friend class CommandBuffer;

private:
    vk::Buffer _buffer;
    vk::DeviceSize _size;
    MemoryAllocator::Allocation _memory;

    Buffer(const std::string &resourceName, const std::string &deviceName, vk::DeviceSize size, vk::BufferUsageFlags usage, void *pData = nullptr);
    ~Buffer() override;
    MemoryAllocator::Allocation createBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties, vk::Buffer &buffer);
    uint32_t findMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void copyBuffer(vk::Buffer &src, vk::Buffer &dst);
    const vk::Buffer &buffer() const;

public:
    void updateBufferData(const void *pData) const;
    vk::DeviceSize size() const;
};
} // namespace Vkbase