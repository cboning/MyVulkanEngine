#pragma once
#include <functional>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>


namespace Vkbase
{
class GpuResourceGarbageCollector
{
public:
    GpuResourceGarbageCollector() = default;
    ~GpuResourceGarbageCollector();

    void defer(std::unique_ptr<uint32_t> &counter, std::function<void()> deleter);

    void collect();

    void forceCollect();

    bool hasPending() const;

private:
    std::vector<std::pair<std::unique_ptr<uint32_t>, std::function<void()>>> _pending;
};

} // namespace Vkbase
