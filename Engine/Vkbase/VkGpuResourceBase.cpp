#include "VkGpuResourceBase.h"
#include "Device.h"

namespace Vkbase
{
VkGpuResourceBase::VkGpuResourceBase(VkResourceType type, const std::string &name, const VkResourceManagerHolder::WeakReference &device)
    : VkResourceBase(type, name), _device(connectTo(std::move(device)))
{
}

VkGpuResourceBase::~VkGpuResourceBase()
{
    for (auto pExtraSuperresource : _pExtraSuperresources)
        if (auto p = pExtraSuperresource.lock<VkGpuResourceBase>())
            p->_pExtraSubresources.erase(weakReference());
    for (auto pExtraSubresource : _pExtraSubresources)
        if (auto p = pExtraSubresource.lock<VkGpuResourceBase>())
            p->_pExtraSuperresources.erase(weakReference());

    if (!_pendingDestroy)
        deferDestroy();
}

VkResourceManagerHolder::WeakReference &VkGpuResourceBase::device() { return _device; }

bool VkGpuResourceBase::isPendingDestroy() const { return _pendingDestroy; }

bool VkGpuResourceBase::inUse() const { return *_submitCounter; }

std::unordered_set<uint32_t *> VkGpuResourceBase::counters()
{
    std::unordered_set<uint32_t *> collectedCounters;
    std::unordered_set<VkResourceManagerHolder::WeakReference> visited;

    std::function<void(const VkResourceManagerHolder::WeakReference &)> collect = [&](const VkResourceManagerHolder::WeakReference &pResource)
    {
        if (!visited.insert(pResource).second)
            return;
        if (auto p = pResource.lock<VkGpuResourceBase>())
        {
            collectedCounters.insert(p->_submitCounter.get());
            for (auto s : p->_pSuperresources)
                collect(s->weakReference());
        }
    };

    collect(weakReference());
    for (auto pExtraSubresource : _pExtraSubresources)
        if (auto p = pExtraSubresource.lock<VkGpuResourceBase>())
            for (uint32_t *pCounter : p->counters())
                collectedCounters.insert(pCounter);
    return collectedCounters;
}

void VkGpuResourceBase::deferDestroy()
{
    if (auto p = _device.lock<Device>())
    {
        if (_pendingDestroy)
            return;
        _pendingDestroy = true;
        p->gpuResourceGarbageCollector().defer(_submitCounter, _onDelayDestroy);
    }
}

void VkGpuResourceBase::addExtraSubresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    if (auto p = pResource.lock<VkGpuResourceBase>())
    {
        _pExtraSubresources.insert(pResource);
        p->_pExtraSuperresources.insert(weakReference());
    }
}

void VkGpuResourceBase::removeExtraSubresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    if (auto p = pResource.lock<VkGpuResourceBase>())
    {
        _pExtraSubresources.erase(pResource);
        p->_pExtraSuperresources.erase(weakReference());
    }
}

} // namespace Vkbase
