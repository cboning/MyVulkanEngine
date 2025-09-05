#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
class Device;
class Buffer : public ResourceBase
{
    friend class ResourceManager;

  private:
    const Device &_device;
    vk::DeviceMemory _memory;
    vk::Buffer _buffer;
    vk::DeviceSize _size;

    Buffer(const std::string &resourceName, const std::string &deviceName, vk::DeviceSize size, vk::BufferUsageFlags usage, void *pData = nullptr);
    ~Buffer() override;
    void createBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties, vk::Buffer &buffer, vk::DeviceMemory &memory);
    uint32_t findMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void copyBuffer(vk::Buffer &src, vk::Buffer &dst);

  public:
    const vk::Buffer &buffer() const;
    void updateBufferData(const void *pData) const;
};
} // namespace Vkbase