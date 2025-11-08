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
    std::unordered_set<VkResourceManagerHolder::WeakReference> _resources;
    std::function<void()> _destroyCallback = {};
    bool _destroying = false;

    void removeResource(const VkResourceManagerHolder::WeakReference &pResource);

protected:
    template <typename T, typename... Args> VkResourceManagerHolder::WeakReference createResource(Args &&...args);
};

template <typename T, typename... Args> inline VkResourceManagerHolder::WeakReference VkResourcesDelegator::createResource(Args &&...args)
{
    VkResourceManagerHolder::WeakReference resource = VkResourceBase::resourceManager().create<T>(args...);
    _resources.insert(resource);
    if (auto p = resource.lock())
        p->_pResourcesDelegator = this;

    return resource;
}
} // namespace Vkbase