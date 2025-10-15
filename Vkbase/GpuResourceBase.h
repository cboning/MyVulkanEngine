#pragma once
#include "ResourceBase.h"
#include <functional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class Device;

class GpuResourceBase : public ResourceBase
{
    friend class CommandBuffer;

private:
    std::unique_ptr<uint32_t> _submitCounter = std::make_unique<uint32_t>(0);
    std::unordered_set<uint32_t *> counters();
    std::unordered_set<GpuResourceBase *> _pExtraSubresources;
    std::unordered_set<GpuResourceBase *> _pExtraSuperresources;

protected:
    Device &_device;
    bool _pendingDestroy = false;
    std::function<void()> _onDelayDestroy;
    void deferDestroy();
    void addExtraSubresource(GpuResourceBase *pResource);
    void removeExtraSubresource(GpuResourceBase *pResource);

public:
    GpuResourceBase(ResourceType type, const std::string &name, Device &device);
    virtual ~GpuResourceBase();

    bool isPendingDestroy() const;
};
} // namespace Vkbase
