#include "ResourceBase.h"
#include <iostream>

namespace Vkbase
{
    ResourceBase::ResourceBase(ResourceType resourceType, const std::string &resourceName)
        : _name(resourceName.empty() ? std::to_string(_nameId++) : resourceName), _resourceType(resourceType)
    {
        _resourceManager.addResource(this);
        std::cout << "[Info] " << toString(resourceType) << " Resource " << _name << " created." << std::endl;
    }

    ResourceBase::~ResourceBase()
    {
#ifdef DEBUG
        std::cout << "[Info] Success to remove the resource. Type: " << toString(_resourceType) << ", Name: " << _name << std::endl;
#endif
        // std::cout << "Destroy the Resource. Type: " << toString(_resourceType) << " Name: " << _name << std::endl;
        for (std::reverse_iterator<std::vector<Vkbase::ResourceBase *>::iterator> iter = _pSubresources.rbegin(); iter != _pSubresources.rend(); ++iter)
            (*iter)->disuseSuperresource(this);
        
    }

    void ResourceBase::preDestroy()
    {
        // auto resourceManager = _resourceManager;
        for (std::reverse_iterator<std::vector<Vkbase::ResourceBase *>::iterator> iter = _pSuperresources.rbegin(); iter != _pSuperresources.rend(); ++iter)
            (*iter)->disusedSubresource(this);
    }

    ResourceManager &ResourceBase::resourceManager()
    {
        return _resourceManager;
    }

    void ResourceBase::useSuperresource(ResourceBase *pResource)
    {
        if (std::find(_pSuperresources.begin(), _pSuperresources.end(), pResource) != _pSuperresources.end())
            return ;
        
        _pSuperresources.push_back(pResource);
    }

    void ResourceBase::useSubresource(ResourceBase *pResource)
    {
        if (std::find(_pSubresources.begin(), _pSubresources.end(), pResource) != _pSubresources.end())
            return ;
        
        _pSubresources.push_back(pResource);
    }

    void ResourceBase::disusedSubresource(ResourceBase *pResource)
    {
        std::vector<ResourceBase *>::iterator iter = std::find(_pSubresources.begin(), _pSubresources.end(), pResource);
        _pSubresources.erase(iter);
        destroy();
    }

    void ResourceBase::disuseSuperresource(ResourceBase *pResource)
    {
        std::vector<ResourceBase *>::iterator iter = std::find(_pSuperresources.begin(), _pSuperresources.end(), pResource);
        _pSuperresources.erase(iter);
        if (_pSuperresources.empty() && !_locked)
            destroy();
    }

    const std::string &ResourceBase::name() const
    {
        return _name;
    }

    const ResourceType &ResourceBase::type() const
    {
        return _resourceType;
    }

    void ResourceBase::destroy() const
    {
        _resourceManager.remove(_resourceType, _name);
    }

    void ResourceBase::rename(const std::string &name)
    {
        _name = name;
        _resourceManager.addResource(this);
    }

    void ResourceBase::setLock()
    {
        _locked = true;
    }

    void ResourceBase::setUnlock()
    {
        _locked = false;
    }
}