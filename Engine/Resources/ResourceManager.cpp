#include "ResourceManager.h"
#include "ResourceBase.h"

namespace Resources
{
ResourceManager &ResourceManager::instance()
{
    static ResourceManager instance;
    return instance;
}

ResourceManager::~ResourceManager()
{
    while (_pResources.size())
        removeResource(_pResources.begin()->second);
}

void ResourceManager::addResource(ResourceBase *pResource) { _pResources.insert({pResource->name(), pResource}); }

void ResourceManager::removeResource(ResourceBase *pResource)
{
    ResourceBase *resource = _pResources.extract(pResource->name()).mapped();
    if (resource)
        delete resource;
}

} // namespace Resources
