#pragma once
#include <memory>
#include <functional>


namespace Vkbase
{
class VkResourceBase;

class VkResourceManagerHolder
{
    friend class VkResourceManager;

private:
    VkResourceBase *_pResource;
    std::shared_ptr<uint32_t> _weakReferenceCount = std::make_shared<uint32_t>(0);
    std::shared_ptr<bool> _expired = std::make_shared<bool>(false);

    VkResourceManagerHolder(VkResourceBase *pResource) noexcept;
    
    static VkResourceManagerHolder create(VkResourceBase *pResource) noexcept;

public:
    class WeakReference
    {
    private:
        std::shared_ptr<uint32_t> _weakReferenceCount;
        std::shared_ptr<bool> _expired;
        VkResourceBase *_pResource = nullptr;

    public:
        WeakReference(const VkResourceManagerHolder &holder) noexcept;
        WeakReference() noexcept;

        WeakReference(const WeakReference &other) noexcept;
        WeakReference &operator=(const WeakReference &other) noexcept;
        WeakReference(WeakReference &&other) noexcept;
        WeakReference &operator=(WeakReference &&other) noexcept;
        bool operator==(const WeakReference &other) const;
        bool operator!=(const WeakReference &other) const;
        ~WeakReference();
        VkResourceBase *lock() const noexcept;
        template <typename T> T *lock() const noexcept;
    };
    friend class WeakReference;

    VkResourceManagerHolder(const VkResourceManagerHolder &) = delete;
    VkResourceManagerHolder &operator=(const VkResourceManagerHolder &) = delete;
    VkResourceManagerHolder(VkResourceManagerHolder &&other) = default;
    VkResourceManagerHolder &operator=(VkResourceManagerHolder &&other) = default;

    
    ~VkResourceManagerHolder();
};

template <typename T> inline T *VkResourceManagerHolder::WeakReference::lock() const noexcept
{
    if (auto p = lock())
        return dynamic_cast<T *>(p);
    return nullptr;
}
} // namespace Vkbase

namespace std {
template <>
struct hash<Vkbase::VkResourceManagerHolder::WeakReference> {
    std::size_t operator()(const Vkbase::VkResourceManagerHolder::WeakReference &ref) const noexcept {
        return std::hash<void*>{}(ref.lock());
    }
};
} // namespace std