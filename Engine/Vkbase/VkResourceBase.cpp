#include "VkResourceBase.h"
#include "VkResourcesDelegator.h"
#include <iostream>

namespace Vkbase
{
VkResourceBase::VkResourceBase(VkResourceType resourceType, const std::string &resourceName)
    : _name(getSuitableName(resourceType, resourceName)), _resourceType(resourceType)
{
    _resourceManager.addResource(this);
#ifdef DEBUG
    std::cout << "[Info] " << toString(resourceType) << " Resource " << _name << " created." << std::endl;
#endif
}

VkResourceBase::~VkResourceBase()
{
#ifdef DEBUG
    std::cout << "[Info] Success to remove the resource. Type: " << toString(_resourceType) << ", Name: " << _name << std::endl;
#endif
    if (_pResourcesDelegator)
        _pResourcesDelegator->removeResource(weakReference());
    _killing = true;
    if (_resourceManager.resource(_resourceType, _name).lock())
        destroy();
    for (std::reverse_iterator<std::vector<VkResourceManagerHolder::WeakReference>::iterator> iter = _pSubresources.rbegin(); iter != _pSubresources.rend();
         ++iter)
        if (auto p = iter->lock())
            p->disuseSuperresource(weakReference());
}

void VkResourceBase::preDestroy()
{
    while (_pSuperresources.size())
    {
        VkResourceManagerHolder::WeakReference back = _pSuperresources.back();
        _pSuperresources.pop_back();

        if (auto p = back.lock())
            p->destroySubresource(weakReference());
    }
}

bool VkResourceBase::killingSelf() { return _killing; }

VkResourceManager &VkResourceBase::resourceManager() { return _resourceManager; }

void VkResourceBase::useSuperresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    if (std::find(_pSuperresources.begin(), _pSuperresources.end(), pResource) != _pSuperresources.end())
        return;

    _pSuperresources.push_back(pResource);
}

void VkResourceBase::useSubresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    if (std::find(_pSubresources.begin(), _pSubresources.end(), pResource) != _pSubresources.end())
        return;

    _pSubresources.push_back(pResource);
}

void VkResourceBase::destroySubresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    disuseSubresource(pResource);
    destroy();
}

void VkResourceBase::disuseSubresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    std::vector<VkResourceManagerHolder::WeakReference>::iterator iter = std::find(_pSubresources.begin(), _pSubresources.end(), pResource);
    if (iter == _pSubresources.end())
        return;
    _pSubresources.erase(iter);
}

void VkResourceBase::disuseSuperresource(const VkResourceManagerHolder::WeakReference &pResource)
{
    std::vector<VkResourceManagerHolder::WeakReference>::iterator iter = std::find(_pSuperresources.begin(), _pSuperresources.end(), pResource);
    if (iter == _pSuperresources.end())
        return;
    _pSuperresources.erase(iter);
    if (_pSuperresources.empty() && !_locked)
        destroy();
}

const std::string &VkResourceBase::name() const { return _name; }

const VkResourceType &VkResourceBase::type() const { return _resourceType; }

void VkResourceBase::destroy() const { _resourceManager.remove(_resourceType, _name); }

VkResourceManagerHolder::WeakReference VkResourceBase::weakReference() const { return _resourceManager.resource(_resourceType, _name); }

void VkResourceBase::setLock() { _locked = true; }

void VkResourceBase::setUnlock() { _locked = false; }

std::string VkResourceBase::getSuitableName(const VkResourceType &type, std::string name)
{
    if (!name.empty())
        return name;

    name = toString(type);

    if (_resourceManager.resource(type, name).lock())
        return name;

    uint32_t index = 0;
    while (true)
    {
        std::string currentName = name + "_" + std::to_string(++index);
        if (!_resourceManager.resource(type, currentName).lock())
            return currentName;
    }
}
VkResourceManagerHolder::WeakReference &&VkResourceBase::connectTo(VkResourceManagerHolder::WeakReference &&pResource)
{
    Vkbase::VkResourceManagerHolder::WeakReference thisReference = weakReference();
    if (pResource == thisReference)
        return std::forward<VkResourceManagerHolder::WeakReference>(pResource);
    if (auto p = pResource.lock())
        p->useSuperresource(thisReference);
    else
        throw std::runtime_error("Invalid type: not the expected derived class");

    useSubresource(pResource);
    return std::forward<VkResourceManagerHolder::WeakReference>(pResource);
}
VkResourceManagerHolder::WeakReference &&VkResourceBase::disconnectTo(VkResourceManagerHolder::WeakReference &&pResource)
{
    Vkbase::VkResourceManagerHolder::WeakReference thisReference = weakReference();
    if (pResource == thisReference)
        return std::forward<VkResourceManagerHolder::WeakReference>(pResource);

    if (auto p = pResource.lock())
        p->disuseSuperresource(thisReference);
    else
        throw std::runtime_error("Invalid type: not the expected derived class");

    disuseSubresource(pResource);
    return std::forward<VkResourceManagerHolder::WeakReference>(pResource);
}

const VkResourceManagerHolder::WeakReference &VkResourceBase::connectTo(const VkResourceManagerHolder::WeakReference &pResource)
{
    Vkbase::VkResourceManagerHolder::WeakReference thisReference = weakReference();
    if (pResource == thisReference)
        return pResource;
    if (auto p = pResource.lock())
        p->useSuperresource(thisReference);
    else
        throw std::runtime_error("Invalid type: not the expected derived class");

    useSubresource(pResource);
    return pResource;
}

const VkResourceManagerHolder::WeakReference &VkResourceBase::disconnectTo(const VkResourceManagerHolder::WeakReference &pResource)
{
    Vkbase::VkResourceManagerHolder::WeakReference thisReference = weakReference();
    if (pResource == thisReference)
        return pResource;

    if (auto p = pResource.lock())
        p->disuseSuperresource(thisReference);
    else
        throw std::runtime_error("Invalid type: not the expected derived class");

    disuseSubresource(pResource);
    return pResource;
}
} // namespace Vkbase