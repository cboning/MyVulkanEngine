#pragma once
#include "VkResourceManager.h"
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class VkResourcesDelegator;

class VkResourceBase
{
    friend class VkResourceManager;
    friend class VkResourcesDelegator;

private:
    inline static VkResourceManager &_resourceManager = VkResourceManager::instance();
    bool _locked = false;
    bool _killing = false;
    VkResourcesDelegator *_pResourcesDelegator = nullptr;

    void useSuperresource(VkResourceBase *pResource);
    void disuseSuperresource(VkResourceBase *pResource);
    void preDestroy();
    bool killingSelf();

protected:
    VkResourceBase(VkResourceType resourceType, const std::string &resourceName);
    const std::string _name;
    const VkResourceType _resourceType;
    std::vector<VkResourceBase *> _pSubresources;
    std::vector<VkResourceBase *> _pSuperresources;
    void useSubresource(VkResourceBase *pResource);
    void disuseSubresource(VkResourceBase *pResource);
    static std::string getSuitableName(const VkResourceType &type, std::string name);
    template <typename T, typename... Args> static T *createResource(Args &&...args);

    template <typename T> T *connectTo(T *pResource)
    {
        if (pResource == this)
            return pResource;
        VkResourceBase *pBase = (VkResourceBase *)pResource;
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
        VkResourceBase *pBase = (VkResourceBase *)pResource;
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
    virtual ~VkResourceBase();
    static VkResourceManager &resourceManager();
    const std::string &name() const;
    const VkResourceType &type() const;
    void setLock();
    void setUnlock();
    void destroy() const;
};
template <typename T, typename... Args> inline T *VkResourceBase::createResource(Args &&...args) { return resourceManager().create<T>(args...); }
} // namespace Vkbase