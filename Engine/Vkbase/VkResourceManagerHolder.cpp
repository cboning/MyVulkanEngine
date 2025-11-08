#include "VkResourceManagerHolder.h"
#include "VkResourceBase.h"
namespace Vkbase
{
VkResourceManagerHolder::VkResourceManagerHolder(VkResourceBase *pResource) noexcept : _pResource(pResource) {}

VkResourceManagerHolder VkResourceManagerHolder::create(VkResourceBase *pResource) noexcept { return VkResourceManagerHolder(pResource); }

bool VkResourceManagerHolder::WeakReference::operator==(const WeakReference &other) const { return lock() == other.lock(); }
bool VkResourceManagerHolder::WeakReference::operator!=(const WeakReference &other) const { return !operator==(other); }
VkResourceManagerHolder::~VkResourceManagerHolder()
{
    if (_expired)
        *_expired = true;
}

VkResourceManagerHolder::WeakReference::WeakReference(const VkResourceManagerHolder &holder) noexcept
    : _weakReferenceCount(holder._weakReferenceCount), _expired(holder._expired), _pResource(holder._pResource)
{
    if (_weakReferenceCount)
        ++(*_weakReferenceCount);
}

VkResourceManagerHolder::WeakReference::WeakReference() noexcept {}

VkResourceManagerHolder::WeakReference::WeakReference(const WeakReference &other) noexcept
    : _weakReferenceCount(other._weakReferenceCount), _expired(other._expired), _pResource(other._pResource)
{
    if (_weakReferenceCount)
        ++(*_weakReferenceCount);
}

VkResourceManagerHolder::WeakReference &VkResourceManagerHolder::WeakReference::operator=(const WeakReference &other) noexcept
{
    if (this != &other)
    {
        if (_weakReferenceCount && !--(*_weakReferenceCount) && !_expired)
            _pResource->destroy();

        _weakReferenceCount = other._weakReferenceCount;
        _expired = other._expired;
        _pResource = other._pResource;

        if (_weakReferenceCount)
            ++(*_weakReferenceCount);
    }
    return *this;
}

VkResourceManagerHolder::WeakReference::WeakReference(WeakReference &&other) noexcept
    : _weakReferenceCount(other._weakReferenceCount), _expired(other._expired), _pResource(other._pResource)
{
    other._weakReferenceCount = nullptr;
    other._expired = nullptr;
    other._pResource = nullptr;
}

VkResourceManagerHolder::WeakReference &VkResourceManagerHolder::WeakReference::operator=(WeakReference &&other) noexcept
{
    if (this != &other)
    {
        if (_weakReferenceCount && !--(*_weakReferenceCount) && !_expired)
            _pResource->destroy();

        _weakReferenceCount = other._weakReferenceCount;
        _expired = other._expired;
        _pResource = other._pResource;

        other._weakReferenceCount = nullptr;
        other._expired = nullptr;
        other._pResource = nullptr;
    }
    return *this;
}

VkResourceManagerHolder::WeakReference::~WeakReference()
{
    if (!_weakReferenceCount)
        return;

    if (!--(*_weakReferenceCount) && !_expired)
        _pResource->destroy();
}

VkResourceBase *VkResourceManagerHolder::WeakReference::lock() const noexcept
{
    if (!_expired || *_expired)
        return nullptr;

    return _pResource;
}
} // namespace Vkbase