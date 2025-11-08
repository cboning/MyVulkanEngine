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

    void useSuperresource(const VkResourceManagerHolder::WeakReference &pResource);
    void disuseSuperresource(const VkResourceManagerHolder::WeakReference &pResource);
    void preDestroy();
    bool killingSelf();

protected:
    VkResourceBase(VkResourceType resourceType, const std::string &resourceName);
    const std::string _name;
    const VkResourceType _resourceType;
    std::vector<VkResourceManagerHolder::WeakReference> _pSubresources;
    std::vector<VkResourceManagerHolder::WeakReference> _pSuperresources;
    void useSubresource(const VkResourceManagerHolder::WeakReference &pResource);
    void destroySubresource(const VkResourceManagerHolder::WeakReference &pResource);
    void disuseSubresource(const VkResourceManagerHolder::WeakReference &pResource);
    static std::string getSuitableName(const VkResourceType &type, std::string name);
    template <typename T, typename... Args> static VkResourceManagerHolder::WeakReference createResource(Args &&...args);

    VkResourceManagerHolder::WeakReference &&connectTo(VkResourceManagerHolder::WeakReference &&pResource);
    VkResourceManagerHolder::WeakReference &&disconnectTo(VkResourceManagerHolder::WeakReference &&pResource);
    const VkResourceManagerHolder::WeakReference &connectTo(const VkResourceManagerHolder::WeakReference &pResource);
    const VkResourceManagerHolder::WeakReference &disconnectTo(const VkResourceManagerHolder::WeakReference &pResource);

public:
    virtual ~VkResourceBase();
    static VkResourceManager &resourceManager();
    const std::string &name() const;
    const VkResourceType &type() const;
    void setLock();
    void setUnlock();
    void destroy() const;
    VkResourceManagerHolder::WeakReference weakReference() const;
};
template <typename T, typename... Args> inline VkResourceManagerHolder::WeakReference VkResourceBase::createResource(Args &&...args) { return resourceManager().create<T>(args...); }
} // namespace Vkbase