#include "ResourceBase.h"
#include <iostream>


namespace Vkbase
{
    ResourceBase::ResourceBase(ResourceType resourceType, const std::string &resourceName)
        : _name(resourceName), _resourceType(resourceType)
    {
        _pResource.insert(this);
        _resourceManager.addResource(resourceType, resourceName, this);
        std::cout << "[Info] "<< toString(resourceType) <<" Resource " << resourceName << " created." << std::endl;
    }

    ResourceBase::~ResourceBase()
    {
        for (ResourceBase *pSubresource : _pSubresources)
            pSubresource->disuseSuperresource(this);
        
        for (ResourceBase *pSuperresource : _pSuperresources)
            pSuperresource->disusedSubresource(this);

        _pResources.erase(this);
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
}