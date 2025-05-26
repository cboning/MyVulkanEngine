#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{

    enum class ResourceType
    {
        Window,
        Swapchain,
        Device,
        Image,
        Buffer,
        Pipeline,
        Framebuffer,
        CommandPool
    };

    class ResourceBase;

    typedef std::unordered_map<ResourceType, std::unordered_map<std::string, ResourceBase *>> ResourceSet;

    inline std::string toString(ResourceType type)
    {
        switch (type)
        {
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
        }
    }
    class ResourceManager
    {
    private:
        vk::Instance _instance;
        ResourceSet _pResources;

        void createInstance(std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"}, std::vector<const char *> extensions = {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, "VK_EXT_metal_surface", "VK_KHR_surface"}, std::string appName = "Vulkan");
    public:
        ResourceManager();
        ~ResourceManager();
        void addResource(ResourceType type, std::string name, ResourceBase *pResource);
        ResourceSet &resources();
        ResourceBase *resource(ResourceType type, std::string name);
        void remove(ResourceType type, std::string name);
        vk::Instance &instance();
    };
}