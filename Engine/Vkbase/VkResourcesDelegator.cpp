#include "VkResourcesDelegator.h"
#include "VkResourceBase.h"
namespace Vkbase
{
VkResourcesDelegator::~VkResourcesDelegator()
{
    _destroying = true;
    while (_pResources.size())
        (*_pResources.begin())->destroy();
}

VkResourcesDelegator::VkResourcesDelegator(VkResourcesDelegator &&other) noexcept : _destroyCallback(std::move(other._destroyCallback))
{
    _pResources = std::move(other._pResources);

    for (auto *pResource : _pResources)
        pResource->_pResourcesDelegator = this;

    other._pResources.clear();
}

VkResourcesDelegator &VkResourcesDelegator::operator=(VkResourcesDelegator &&other) noexcept
{
    if (this != &other)
    {
        if (_destroyCallback)
            _destroyCallback();

        for (auto *pResource : _pResources)
            pResource->_pResourcesDelegator = nullptr;

        _pResources.clear();

        _pResources = std::move(other._pResources);
        _destroyCallback = std::move(other._destroyCallback);

        for (auto *pResource : _pResources)
            pResource->_pResourcesDelegator = this;

        other._pResources.clear();
    }
    return *this;
}

void VkResourcesDelegator::setDestroyCallback(std::function<void()> destroyCallback) { _destroyCallback = std::move(destroyCallback); }

void VkResourcesDelegator::removeResource(VkResourceBase *pResource)
{
    _pResources.erase(pResource);

    if (!_destroying && _destroyCallback)
    {
        _destroyCallback();
        _destroyCallback = {};
    }
}

void VkResourcesDelegator::addKeyResource(VkResourceBase *pResource)
{
    if (!_pResources.count(pResource))
        throw std::runtime_error("This resource is not create by this delegator.");

    _pKeyResources.insert(pResource);
}
} // namespace Vkbase