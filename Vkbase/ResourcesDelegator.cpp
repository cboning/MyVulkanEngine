#include "ResourcesDelegator.h"
#include "ResourceBase.h"
namespace Vkbase
{
ResourcesDelegator::~ResourcesDelegator()
{
    while (_pResources.size())
        (*_pResources.begin())->destroy();
}
void ResourcesDelegator::removeResource(ResourceBase *pResource) { _pResources.erase(pResource); }
} // namespace Vkbase