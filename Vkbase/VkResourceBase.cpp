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
        _pResourcesDelegator->removeResource(this);
    _killing = true;
    if (_resourceManager.resource(_resourceType, _name))
        destroy();
    for (std::reverse_iterator<std::vector<Vkbase::VkResourceBase *>::iterator> iter = _pSubresources.rbegin(); iter != _pSubresources.rend(); ++iter)
        (*iter)->disuseSuperresource(this);
}

void VkResourceBase::preDestroy()
{
    while (_pSuperresources.size())
    {
        Vkbase::VkResourceBase *back = _pSuperresources.back();
        _pSuperresources.pop_back();
        back->disuseSubresource(this);
    }
}

bool VkResourceBase::killingSelf() { return _killing; }

VkResourceManager &VkResourceBase::resourceManager() { return _resourceManager; }

void VkResourceBase::useSuperresource(VkResourceBase *pResource)
{
    if (std::find(_pSuperresources.begin(), _pSuperresources.end(), pResource) != _pSuperresources.end())
        return;

    _pSuperresources.push_back(pResource);
}

void VkResourceBase::useSubresource(VkResourceBase *pResource)
{
    if (std::find(_pSubresources.begin(), _pSubresources.end(), pResource) != _pSubresources.end())
        return;

    _pSubresources.push_back(pResource);
}

void VkResourceBase::disuseSubresource(VkResourceBase *pResource)
{
    std::vector<VkResourceBase *>::iterator iter = std::find(_pSubresources.begin(), _pSubresources.end(), pResource);
    if (iter == _pSubresources.end())
        return;
    _pSubresources.erase(iter);
    destroy();
}

void VkResourceBase::disuseSuperresource(VkResourceBase *pResource)
{
    std::vector<VkResourceBase *>::iterator iter = std::find(_pSuperresources.begin(), _pSuperresources.end(), pResource);
    if (iter == _pSuperresources.end())
        return;
    _pSuperresources.erase(iter);
    if (_pSuperresources.empty() && !_locked)
        destroy();
}

const std::string &VkResourceBase::name() const { return _name; }

const VkResourceType &VkResourceBase::type() const { return _resourceType; }

void VkResourceBase::destroy() const { _resourceManager.remove(_resourceType, _name); }

void VkResourceBase::setLock() { _locked = true; }

void VkResourceBase::setUnlock() { _locked = false; }

std::string VkResourceBase::getSuitableName(const VkResourceType &type, std::string name)
{
    if (!name.empty())
        return name;

    name = toString(type);

    if (!_resourceManager.resource(type, name))
        return name;

    uint32_t index = 0;
    while (true)
    {
        std::string currentName = name + "_" + std::to_string(++index);
        if (!_resourceManager.resource(type, currentName))
            return currentName;
    }
}
} // namespace Vkbase