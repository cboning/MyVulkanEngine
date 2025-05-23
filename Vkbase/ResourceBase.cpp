#include "ResourceBase.h"
#include <iostream>


namespace Vkbase
{
    ResourceBase::ResourceBase(const std::string &resourceName, ResourceType resourceType)
        : _name(resourceName), _resourceType(resourceType)
    {
        _resourceManager.addResource(resourceName, this);
        std::cout << "[Info] Resource " << resourceName << " created." << std::endl;
    }

    ResourceManager &ResourceBase::resourceManager()
    {
        return _resourceManager;
    }
}