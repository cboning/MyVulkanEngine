#pragma once
#include "VkResourceBase.h"
#include <unordered_set>

namespace Vkbase
{
class VkResourcesDelegator
{
    friend class VkResourceBase;

public:
    VkResourcesDelegator() = default;
    ~VkResourcesDelegator();

    VkResourcesDelegator(const VkResourcesDelegator &) = delete;
    VkResourcesDelegator &operator=(const VkResourcesDelegator &) = delete;
    VkResourcesDelegator(VkResourcesDelegator &&other) noexcept;
    VkResourcesDelegator &operator=(VkResourcesDelegator &&other) noexcept;

    void setDestroyCallback(std::function<void()> destroyCallback);

private:
    std::unordered_set<VkResourceBase *> _pResources;
    std::function<void()> _destroyCallback = {};
    std::unordered_set<VkResourceBase *> _pKeyResources;
    bool _destroying = false;

    void removeResource(VkResourceBase *pResource);

protected:
    template <typename T, typename... Args> T *createResource(Args &&...args);
    void addKeyResource(VkResourceBase *pResource);
};

template <typename T, typename... Args> inline T *VkResourcesDelegator::createResource(Args &&...args)
{
    T *pResource = VkResourceBase::resourceManager().create<T>(args...);
    _pResources.insert(pResource);
    pResource->_pResourcesDelegator = this;

    return pResource;
}
} // namespace Vkbase