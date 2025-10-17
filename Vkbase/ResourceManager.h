#pragma once
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
enum class ResourceType
{
    Unknown,
    Window,
    Swapchain,
    Device,
    Image,
    Buffer,
    Pipeline,
    Framebuffer,
    CommandPool,
    RenderPass,
    Sampler,
    DescriptorSets,
    RenderDelegator,
    CommandBuffer
};

class ResourceBase;

typedef std::unordered_map<ResourceType, std::unordered_map<std::string, ResourceBase *>> ResourceSet;

inline std::string toString(ResourceType type)
{
    switch (type)
    {
    case ResourceType::Unknown:
        return "Unknown";
    case ResourceType::Window:
        return "Window";
    case ResourceType::Swapchain:
        return "Swapchain";
    case ResourceType::Device:
        return "Device";
    case ResourceType::Image:
        return "Image";
    case ResourceType::Buffer:
        return "Buffer";
    case ResourceType::Pipeline:
        return "Pipeline";
    case ResourceType::Framebuffer:
        return "Framebuffer";
    case ResourceType::CommandPool:
        return "CommandPool";
    case ResourceType::RenderPass:
        return "RenderPass";
    case ResourceType::Sampler:
        return "Sampler";
    case ResourceType::DescriptorSets:
        return "DescriptorSets";
    case ResourceType::RenderDelegator:
        return "RenderDelegator";
    case ResourceType::CommandBuffer:
        return "CommandBuffer";
    }
}
class ResourceManager
{
    friend class ResourceBase;
    friend class ResourcesDelegator;

private:
    vk::Instance _instance;
    ResourceSet _pResources;

    void createInstance(std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"},
                        std::vector<const char *> extensions = {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef __APPLE__
                                                                "VK_EXT_metal_surface"
#endif
                        },
                        std::string appName = "Vulkan");
    void addResource(ResourceBase *pResource);
    template <typename T, typename... Args> T *create(Args &&...args);
    ResourceManager();
    ~ResourceManager();

public:
    const ResourceSet &resources() const;

    ResourceBase *resource(ResourceType type, const std::string &name) const;
    const vk::Instance &vkInstance() const;
    static ResourceManager &instance();
    void remove(ResourceType type, const std::string &name);
};

template <typename T, typename... Args> T *ResourceManager::create(Args &&...args) { return new T(args...); }
} // namespace Vkbase