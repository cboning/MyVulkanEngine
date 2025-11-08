#include "VkGpuResourceBase.h"
#include "Device.h"

namespace Vkbase
{
VkGpuResourceBase::VkGpuResourceBase(VkResourceType type, const std::string &name, Device &device) : VkResourceBase(type, name), _device(*connectTo(&device)) {}

VkGpuResourceBase::~VkGpuResourceBase()
{
    for (auto *pExtraSuperresource : _pExtraSuperresources)
        pExtraSuperresource->_pExtraSubresources.erase(this);
    for (auto *pExtraSubresource : _pExtraSubresources)
        pExtraSubresource->_pExtraSuperresources.erase(this);

    if (!_pendingDestroy)
        deferDestroy();
}

Device &VkGpuResourceBase::device() { return _device; }

bool VkGpuResourceBase::isPendingDestroy() const { return _pendingDestroy; }

std::unordered_set<uint32_t *> VkGpuResourceBase::counters()
{
    std::unordered_set<uint32_t *> collectedCounters;
    std::unordered_set<const VkGpuResourceBase *> visited;

    std::function<void(const VkGpuResourceBase *)> collect = [&](const VkGpuResourceBase *pResource)
    {
        if (!pResource || !visited.insert(pResource).second)
            return;
        collectedCounters.insert(pResource->_submitCounter.get());
        for (auto *s : pResource->_pSuperresources)
            if (auto *gpuRes = dynamic_cast<VkGpuResourceBase *>(s))
                collect(gpuRes);
    };

    collect(this);
    for (auto *pExtraSubresource : _pExtraSubresources)
        for (uint32_t *pCounter : pExtraSubresource->counters())
            collectedCounters.insert(pCounter);
    return collectedCounters;
}

void VkGpuResourceBase::deferDestroy()
{
    if (_pendingDestroy)
        return;
    _pendingDestroy = true;
    _device.gpuResourceGarbageCollector().defer(_submitCounter, _onDelayDestroy);
}

void VkGpuResourceBase::addExtraSubresource(VkGpuResourceBase *pResource)
{
    _pExtraSubresources.insert(pResource);
    pResource->_pExtraSuperresources.insert(this);
}

void VkGpuResourceBase::removeExtraSubresource(VkGpuResourceBase *pResource)
{
    _pExtraSubresources.erase(pResource);
    pResource->_pExtraSuperresources.erase(this);
}

} // namespace Vkbase
