#pragma once
#include "ResourceBase.h"
#include <unordered_set>

namespace Vkbase
{
class ResourcesDelegator
{
    friend class ResourceBase;

public:
    ResourcesDelegator() = default;
    ~ResourcesDelegator();

    ResourcesDelegator(const ResourcesDelegator &) = delete;
    ResourcesDelegator &operator=(const ResourcesDelegator &) = delete;

private:
    std::unordered_set<ResourceBase *> _pResources;
    void removeResource(ResourceBase *pResource);

protected:
    template <typename T, typename... Args> T *createResource(Args &&...args);
};
template <typename T, typename... Args> inline T *ResourcesDelegator::createResource(Args &&...args)
{
    T *pResource = ResourceBase::resourceManager().create<T>(args...);
    _pResources.insert(pResource);
    pResource->_pResourcesDelegator = this;

    return pResource;
}
} // namespace Vkbase