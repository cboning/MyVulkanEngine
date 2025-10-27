#pragma once
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
enum class VkResourceType
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

class VkResourceBase;

typedef std::unordered_map<VkResourceType, std::unordered_map<std::string, VkResourceBase *>> VkResourceSet;

inline std::string toString(VkResourceType type)
{
    switch (type)
    {
    case VkResourceType::Unknown:
        return "Unknown";
    case VkResourceType::Window:
        return "Window";
    case VkResourceType::Swapchain:
        return "Swapchain";
    case VkResourceType::Device:
        return "Device";
    case VkResourceType::Image:
        return "Image";
    case VkResourceType::Buffer:
        return "Buffer";
    case VkResourceType::Pipeline:
        return "Pipeline";
    case VkResourceType::Framebuffer:
        return "Framebuffer";
    case VkResourceType::CommandPool:
        return "CommandPool";
    case VkResourceType::RenderPass:
        return "RenderPass";
    case VkResourceType::Sampler:
        return "Sampler";
    case VkResourceType::DescriptorSets:
        return "DescriptorSets";
    case VkResourceType::RenderDelegator:
        return "RenderDelegator";
    case VkResourceType::CommandBuffer:
        return "CommandBuffer";
    }
}
class VkResourceManager
{
    friend class VkResourceBase;
    friend class VkResourcesDelegator;

private:
    vk::Instance _instance;
    VkResourceSet _pResources;

    void createInstance(std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"},
                        std::vector<const char *> extensions = {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef __APPLE__
                                                                "VK_EXT_metal_surface"
#endif
                        },
                        std::string appName = "Vulkan");
    void addResource(VkResourceBase *pResource);
    template <typename T, typename... Args> T *create(Args &&...args);
    VkResourceManager();
    ~VkResourceManager();

public:
    const VkResourceSet &resources() const;

    VkResourceBase *resource(VkResourceType type, const std::string &name) const;
    const vk::Instance &vkInstance() const;
    static VkResourceManager &instance();
    void remove(VkResourceType type, const std::string &name);
};

template <typename T, typename... Args> T *VkResourceManager::create(Args &&...args) { return new T(args...); }
} // namespace Vkbase