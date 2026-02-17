#pragma once
#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{

struct MemoryBlock
{
    vk::DeviceMemory memory;
    vk::DeviceSize size = 0;
    uint32_t memoryTypeIndex = 0;

    std::map<vk::DeviceSize, vk::DeviceSize> freeRanges;

    vk::DeviceSize cursor = 0;
};

class MemoryAllocator
{
public:
    class Allocation
    {
        friend class MemoryAllocator;

    public:
        vk::DeviceMemory memory();
        vk::DeviceSize offset();
        vk::DeviceSize size();
        uint32_t memoryTypeIndex();

    private:
        vk::DeviceMemory _memory;
        vk::DeviceSize _offset = 0;
        vk::DeviceSize _size = 0;
        uint32_t _memoryTypeIndex = 0;
    };

    MemoryAllocator(vk::Device device, vk::PhysicalDevice physical, vk::DeviceSize defaultBlockSize = 4ull * 1024 * 1024);
    ~MemoryAllocator();

    MemoryAllocator(const MemoryAllocator &) = delete;
    MemoryAllocator &operator=(const MemoryAllocator &) = delete;

    std::optional<Allocation> allocate(vk::DeviceSize size, vk::DeviceSize alignment, uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties);

    void free(const Allocation &alloc);

    void *map(const Allocation &alloc);
    void unmap(const Allocation &alloc);

    vk::DeviceSize defaultBlockSize() const { return _defaultBlockSize; }

private:
    vk::Device _device;
    vk::PhysicalDevice _physical;
    vk::PhysicalDeviceMemoryProperties _memProps;
    vk::DeviceSize _defaultBlockSize;

    std::mutex _mutex;

    std::vector<std::vector<std::unique_ptr<MemoryBlock>>> _pools;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    MemoryBlock *createBlock(uint32_t memoryTypeIndex, vk::DeviceSize minSize);

    std::pair<MemoryBlock *, vk::DeviceSize> findBlockForAllocation(uint32_t memoryTypeIndex, vk::DeviceSize size, vk::DeviceSize alignment);

    static vk::DeviceSize alignUp(vk::DeviceSize v, vk::DeviceSize alignment) { return (v + (alignment - 1)) & ~(alignment - 1); }
};
} // namespace Vkbase