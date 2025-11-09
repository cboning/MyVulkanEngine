#include "VkResourcesDelegator.h"
#include "VkResourceBase.h"
namespace Vkbase
{
VkResourcesDelegator::~VkResourcesDelegator()
{
    _destroying = true;
    std::unique_lock lock(_controlMutex);
    for (auto resource : _resources)
        if (auto p = resource.lock())
            p->_pResourcesDelegator = nullptr;
    lock.unlock();
    _resources.clear();
}

VkResourcesDelegator::VkResourcesDelegator(VkResourcesDelegator &&other) noexcept : _destroyCallback(std::move(other._destroyCallback))
{
    std::lock_guard lock(_controlMutex);
    _resources = std::move(other._resources);

    for (auto resource : _resources)
        if (auto p = resource.lock())
            p->_pResourcesDelegator = this;

    other._resources.clear();
}

VkResourcesDelegator &VkResourcesDelegator::operator=(VkResourcesDelegator &&other) noexcept
{
    std::lock_guard lock(_controlMutex);
    if (this != &other)
    {
        if (_destroyCallback)
            _destroyCallback();

        for (auto resource : _resources)
            if (auto p = resource.lock())
                p->_pResourcesDelegator = nullptr;

        _resources.clear();

        _resources = std::move(other._resources);
        _destroyCallback = std::move(other._destroyCallback);

        for (auto resource : _resources)
            if (auto p = resource.lock())
                p->_pResourcesDelegator = this;

        other._resources.clear();
    }
    return *this;
}

void VkResourcesDelegator::setDestroyCallback(std::function<void()> destroyCallback) { _destroyCallback = std::move(destroyCallback); }

void VkResourcesDelegator::removeResource(const VkResourceManagerHolder::WeakReference &pResource)
{
    std::lock_guard lock(_controlMutex);
    // find the weak reference that points to pResource and erase it by iterator
    for (auto it = _resources.begin(); it != _resources.end(); ++it)
    {
        if (*it == pResource)
        {
            _resources.erase(it);
            break;
        }
    }

    if (!_destroying && _destroyCallback)
    {
        _destroyCallback();
        _destroyCallback = {};
    }
}
} // namespace Vkbase