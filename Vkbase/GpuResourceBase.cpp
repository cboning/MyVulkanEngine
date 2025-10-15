#include "GpuResourceBase.h"
#include "Device.h"

namespace Vkbase
{
GpuResourceBase::GpuResourceBase(ResourceType type, const std::string &name, Device &device) : ResourceBase(type, name), _device(*connectTo(&device)) {}

GpuResourceBase::~GpuResourceBase()
{
    for (auto *pExtraSuperresource : _pExtraSuperresources)
        pExtraSuperresource->removeExtraSubresource(this);
    for (auto *pExtraSubresource : _pExtraSubresources)
        removeExtraSubresource(pExtraSubresource);

    if (!_pendingDestroy)
        deferDestroy();
}

bool GpuResourceBase::isPendingDestroy() const { return _pendingDestroy; }

std::unordered_set<uint32_t *> GpuResourceBase::counters()
{
    std::unordered_set<uint32_t *> collectedCounters;
    std::unordered_set<const GpuResourceBase *> visited;

    std::function<void(const GpuResourceBase *)> collect = [&](const GpuResourceBase *pResource)
    {
        if (!pResource || !visited.insert(pResource).second)
            return;
        collectedCounters.insert(pResource->_submitCounter.get());
        for (auto *s : pResource->_pSuperresources)
            if (auto *gpuRes = dynamic_cast<GpuResourceBase *>(s))
                collect(gpuRes);
    };

    collect(this);
    for (auto *pExtraSubresource : _pExtraSubresources)
        for (uint32_t *pCounter : pExtraSubresource->counters())
            collectedCounters.insert(pCounter);
    return collectedCounters;
}

void GpuResourceBase::deferDestroy()
{
    if (_pendingDestroy)
        return;
    _pendingDestroy = true;
    _device.gpuResourceGarbageCollector().defer(_submitCounter, _onDelayDestroy);
}

void GpuResourceBase::addExtraSubresource(GpuResourceBase *pResource)
{
    _pExtraSubresources.insert(pResource);
    pResource->_pExtraSuperresources.insert(this);
}

void GpuResourceBase::removeExtraSubresource(GpuResourceBase *pResource)
{
    _pExtraSubresources.erase(pResource);
    pResource->_pExtraSuperresources.erase(this);
}

} // namespace Vkbase
