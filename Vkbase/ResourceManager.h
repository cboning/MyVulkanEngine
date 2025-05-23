#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
    class ResourceBase;

    typedef std::unordered_map<std::string, ResourceBase *> ResourceSet;

    class ResourceManager
    {
    private:
        vk::Instance _instance;
        ResourceSet _resources;

        void createInstance(std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"}, std::vector<const char *> extensions = {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, "VK_EXT_metal_surface", "VK_KHR_surface"}, std::string appName = "Vulkan");
    public:
        ResourceManager();
        ~ResourceManager();
        void addResource(std::string name, ResourceBase *pResource);
        ResourceSet &resources();
        void remove(std::string name);
        vk::Instance &instance();
    };
}