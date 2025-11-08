#pragma once
#include "VkResourceBase.h"
#include <functional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class Device;

class VkGpuResourceBase : public VkResourceBase
{
    friend class CommandBuffer;
private:
    std::unique_ptr<uint32_t> _submitCounter = std::make_unique<uint32_t>(0);
    std::unordered_set<uint32_t *> counters();
    std::unordered_set<VkResourceManagerHolder::WeakReference> _pExtraSubresources;
    std::unordered_set<VkResourceManagerHolder::WeakReference> _pExtraSuperresources;

protected:
    VkResourceManagerHolder::WeakReference _device;
    bool _pendingDestroy = false;
    std::function<void()> _onDelayDestroy;
    void deferDestroy();
    void addExtraSubresource(const VkResourceManagerHolder::WeakReference &pResource);
    void removeExtraSubresource(const VkResourceManagerHolder::WeakReference &pResource);

public:
    VkGpuResourceBase(VkResourceType type, const std::string &name, const VkResourceManagerHolder::WeakReference &device);
    virtual ~VkGpuResourceBase();

    VkResourceManagerHolder::WeakReference &device();
    bool isPendingDestroy() const;
};
} // namespace Vkbase
