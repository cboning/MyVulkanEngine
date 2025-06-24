#include "ResourceBase.h"
#include <iostream>


namespace Vkbase
{
    ResourceBase::ResourceBase(ResourceType resourceType, const std::string &resourceName)
        : _name(resourceName.empty() ? std::to_string(_nameId++) : resourceName), _resourceType(resourceType)
    {\
        _resourceManager.addResource(resourceType, _name, this);
        std::cout << "[Info] " << toString(resourceType) << " Resource " << _name << " created." << std::endl;
    }

    ResourceBase::~ResourceBase()
    {
    }

    ResourceManager &ResourceBase::resourceManager()
    {
        return _resourceManager;
    }

    void ResourceBase::useSuperresource(ResourceBase *pResource)
    {
        _pSuperresources.insert(pResource);
    }

    void ResourceBase::useSubresource(ResourceBase *pResource)
    {
        _pSubresources.insert(pResource);
    }

    void ResourceBase::disusedSubresource(ResourceBase *pResource)
    {
        _pSubresources.erase(pResource);
    }

    void ResourceBase::disuseSuperresource(ResourceBase *pResource)
    {
        _pSuperresources.erase(pResource);
        if (_pSuperresources.empty())
            _resourceManager.remove(_resourceType, _name);
    }

    const std::string &ResourceBase::name() const
    {
        return _name;
    }

    const ResourceType &ResourceBase::type() const
    {
        return _resourceType;
    }

    void ResourceBase::disconnect()
    {
        for (ResourceBase *pSubresource : _pSubresources)
            pSubresource->disuseSuperresource(this);
        
        for (ResourceBase *pSuperresource : _pSuperresources)
            pSuperresource->disusedSubresource(this);
    }
}