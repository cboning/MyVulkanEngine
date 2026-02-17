#include "MemoryAllocator.h"

namespace Vkbase
{
MemoryAllocator::MemoryAllocator(vk::Device device, vk::PhysicalDevice physical, vk::DeviceSize defaultBlockSize)
    : _device(device), _physical(physical), _defaultBlockSize(defaultBlockSize)
{
    _physical.getMemoryProperties(&_memProps);
    _pools.resize(_memProps.memoryTypeCount);
}

MemoryAllocator::~MemoryAllocator()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &vec : _pools)
    {
        for (auto &blockPtr : vec)
        {
            if (blockPtr && blockPtr->memory)
            {
                _device.freeMemory(blockPtr->memory);
            }
        }
        vec.clear();
    }
}

uint32_t MemoryAllocator::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    for (uint32_t i = 0; i < _memProps.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1u << i)) && ((_memProps.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    throw std::runtime_error("Failed to find suitable memory type.");
}

MemoryBlock *MemoryAllocator::createBlock(uint32_t memoryTypeIndex, vk::DeviceSize minSize)
{
    vk::DeviceSize blockSize = std::max(minSize, _defaultBlockSize);
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(blockSize).setMemoryTypeIndex(memoryTypeIndex);

    vk::DeviceMemory mem = _device.allocateMemory(allocInfo);

    auto block = std::make_unique<MemoryBlock>();
    block->memory = mem;
    block->size = blockSize;
    block->memoryTypeIndex = memoryTypeIndex;
    block->cursor = 0;
    block->freeRanges.clear();

    auto ptr = block.get();
    _pools[memoryTypeIndex].push_back(std::move(block));
    return ptr;
}

std::pair<MemoryBlock *, vk::DeviceSize> MemoryAllocator::findBlockForAllocation(uint32_t memoryTypeIndex, vk::DeviceSize size, vk::DeviceSize alignment)
{
    auto &pool = _pools[memoryTypeIndex];
    // first search freeRanges in existing blocks
    for (auto &blockPtr : pool)
    {
        // search freeRanges map for a range with enough size and alignment
        for (auto it = blockPtr->freeRanges.begin(); it != blockPtr->freeRanges.end(); ++it)
        {
            vk::DeviceSize offset = it->first;
            vk::DeviceSize rangeSize = it->second;
            vk::DeviceSize alignedOffset = alignUp(offset, alignment);
            vk::DeviceSize padding = alignedOffset - offset;
            if (rangeSize >= padding + size)
            {
                // consume from this free range
                vk::DeviceSize consumeOffset = alignedOffset;
                vk::DeviceSize remainingBefore = padding;
                vk::DeviceSize remainingAfter = rangeSize - (padding + size);

                // erase current and reinsert leftover ranges
                blockPtr->freeRanges.erase(it);
                if (remainingBefore > 0)
                    blockPtr->freeRanges.emplace(offset, remainingBefore);
                if (remainingAfter > 0)
                    blockPtr->freeRanges.emplace(consumeOffset + size, remainingAfter);

                return {blockPtr.get(), consumeOffset};
            }
        }

        // try append at cursor
        vk::DeviceSize alignedCursor = alignUp(blockPtr->cursor, alignment);
        if (alignedCursor + size <= blockPtr->size)
        {
            vk::DeviceSize allocOffset = alignedCursor;
            blockPtr->cursor = alignedCursor + size;
            return {blockPtr.get(), allocOffset};
        }
    }

    // no existing block fits
    return {nullptr, 0};
}

std::optional<MemoryAllocator::Allocation> MemoryAllocator::allocate(vk::DeviceSize size, vk::DeviceSize alignment, uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties)
{
    std::lock_guard<std::mutex> lock(_mutex);

    uint32_t memType = findMemoryType(memoryTypeBits, properties);

    // try to find or create block
    auto [block, offset] = findBlockForAllocation(memType, size, alignment);
    if (!block)
    {
        // create new block large enough
        MemoryBlock *newBlock = createBlock(memType, std::max(size + alignment, _defaultBlockSize));
        // allocate at beginning
        vk::DeviceSize alignedOffset = alignUp(newBlock->cursor, alignment);
        if (alignedOffset + size > newBlock->size)
            return std::nullopt; // shouldn't happen
        newBlock->cursor = alignedOffset + size;
        Allocation alloc;
        alloc._memory = newBlock->memory;
        alloc._offset = alignedOffset;
        alloc._size = size;
        alloc._memoryTypeIndex = memType;
        return alloc;
    }

    Allocation alloc;
    alloc._memory = block->memory;
    alloc._offset = offset;
    alloc._size = size;
    alloc._memoryTypeIndex = memType;
    return alloc;
}

void MemoryAllocator::free(const Allocation &alloc)
{
    std::lock_guard<std::mutex> lock(_mutex);
    // find the block owning this memory
    for (auto &vec : _pools)
    {
        for (auto &blockPtr : vec)
        {
            if (blockPtr->memory == alloc._memory)
            {
                // insert free range and try to merge with neighbors
                vk::DeviceSize begin = alloc._offset;
                vk::DeviceSize len = alloc._size;

                // find insertion point
                auto it = blockPtr->freeRanges.lower_bound(begin);

                // try merge with previous
                if (it != blockPtr->freeRanges.begin())
                {
                    auto pit = std::prev(it);
                    vk::DeviceSize pbegin = pit->first;
                    vk::DeviceSize plen = pit->second;
                    if (pbegin + plen == begin)
                    {
                        // merge into previous
                        begin = pbegin;
                        len += plen;
                        it = blockPtr->freeRanges.erase(pit);
                    }
                }

                // try merge with next (current it)
                if (it != blockPtr->freeRanges.end())
                {
                    vk::DeviceSize nbegin = it->first;
                    vk::DeviceSize nlen = it->second;
                    if (begin + len == nbegin)
                    {
                        len += nlen;
                        it = blockPtr->freeRanges.erase(it);
                    }
                }

                // insert merged range
                blockPtr->freeRanges.emplace(begin, len);
                return;
            }
        }
    }

    // not found — error
    throw std::runtime_error("Attempt to free allocation from unknown memory block");
}

void *MemoryAllocator::map(const Allocation &alloc)
{
    return _device.mapMemory(alloc._memory, alloc._offset, alloc._size);
}

void MemoryAllocator::unmap(const Allocation &alloc) { _device.unmapMemory(alloc._memory); }

vk::DeviceMemory MemoryAllocator::Allocation::memory() { return _memory; }

vk::DeviceSize MemoryAllocator::Allocation::offset() {
  return _offset;
}

vk::DeviceSize MemoryAllocator::Allocation::size() { return _size; }

uint32_t MemoryAllocator::Allocation::memoryTypeIndex() { return _memoryTypeIndex; }

} // namespace Vkbase
