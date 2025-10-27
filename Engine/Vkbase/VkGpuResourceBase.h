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
    std::unordered_set<VkGpuResourceBase *> _pExtraSubresources;
    std::unordered_set<VkGpuResourceBase *> _pExtraSuperresources;

protected:
    Device &_device;
    bool _pendingDestroy = false;
    std::function<void()> _onDelayDestroy;
    void deferDestroy();
    void addExtraSubresource(VkGpuResourceBase *pResource);
    void removeExtraSubresource(VkGpuResourceBase *pResource);

public:
    VkGpuResourceBase(VkResourceType type, const std::string &name, Device &device);
    virtual ~VkGpuResourceBase();

    Device &device();
    bool isPendingDestroy() const;
};
} // namespace Vkbase
