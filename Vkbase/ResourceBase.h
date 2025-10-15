#pragma once
#include "ResourceManager.h"
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class ResourcesDelegator;

class ResourceBase
{
    friend class ResourceManager;
    friend class ResourcesDelegator;

private:
    inline static ResourceManager &_resourceManager = ResourceManager::instance();
    bool _locked = false;
    bool _killing = false;
    ResourcesDelegator *_pResourcesDelegator = nullptr;

    void useSuperresource(ResourceBase *pResource);
    void disuseSuperresource(ResourceBase *pResource);
    void preDestroy();
    bool killingSelf();

protected:
    ResourceBase(ResourceType resourceType, const std::string &resourceName);
    const std::string _name;
    const ResourceType _resourceType;
    std::vector<ResourceBase *> _pSubresources;
    std::vector<ResourceBase *> _pSuperresources;
    void useSubresource(ResourceBase *pResource);
    void disuseSubresource(ResourceBase *pResource);
    static std::string getSuitableName(const ResourceType &type, std::string name);
    template <typename T, typename... Args> static T *createResource(Args &&...args);

    template <typename T> T *connectTo(T *pResource)
    {
        if (pResource == this)
            return pResource;
        ResourceBase *pBase = (ResourceBase *)pResource;
        if (!pBase)
            throw std::runtime_error("Invalid type: not the expected derived class");
        pBase->useSuperresource(this);
        useSubresource(pBase);
        return pResource;
    }

    template <typename T> T *disconnectTo(T *pResource)
    {
        if (pResource == this)
            return pResource;
        ResourceBase *pBase = (ResourceBase *)pResource;
        if (!pBase)
            throw std::runtime_error("Invalid type: not the expected derived class");
        pBase->disuseSuperresource(this);
        disuseSubresource(pBase);
        return pResource;
    }
    template <typename T> T *checkResource(T *pResource)
    {
        if (pResource)
            return pResource;
        throw std::runtime_error("It porint to an empty resource.");
    }

public:
    virtual ~ResourceBase();
    static ResourceManager &resourceManager();
    const std::string &name() const;
    const ResourceType &type() const;
    void setLock();
    void setUnlock();
    void destroy() const;
};
template <typename T, typename... Args> inline T *ResourceBase::createResource(Args &&...args) { return resourceManager().create<T>(args...); }
} // namespace Vkbase