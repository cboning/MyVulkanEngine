#pragma once
#include <string>
#include <unordered_map>

namespace Resources
{
enum class ResourceType
{
    Unknown,
    Font,
    Texture,
    Model,
    Sampler,
    EmptyTexture
};

class ResourceBase;

class ResourceManager
{
    friend class ResourceBase;

public:
    static ResourceManager &instance();
    static void shutDown();
    template <typename T, typename... Args> T &getResource(Args &&...args) const;
    void removeResource(ResourceBase *pResource);

private:
    std::unordered_map<std::string, ResourceBase *> _pResources;
    inline static ResourceManager *_pInstance = nullptr;

    ~ResourceManager();
    void addResource(ResourceBase *pResource);
};
template <typename T, typename... Args> T &ResourceManager::getResource(Args &&...args) const
{
    std::string name = T::getNameByArgument(args...);
    if (_pResources.count(name))
        return *dynamic_cast<T *>(_pResources.at(name));

    return *(new T(args...));
}
} // namespace Resources