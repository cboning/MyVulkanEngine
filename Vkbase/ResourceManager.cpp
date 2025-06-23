#define DEBUG
#include "ResourceManager.h"
#include "ResourceBase.h"
#include "Window.h"
#include <iostream>

namespace Vkbase
{
    ResourceManager::ResourceManager()
    {
        if (!glfwInit())
        {
            std::cerr << "[Error]Failed to initialize GLFW" << std::endl;
            return;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        createInstance({"VK_LAYER_KHRONOS_validation"}, {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, "VK_MVK_macos_surface"});
    }

    ResourceManager::~ResourceManager()
    {
        _instance.destroy();
    }

    void ResourceManager::createInstance(std::vector<const char *> layers, std::vector<const char *> extensions, std::string appName)
    {
        vk::ApplicationInfo applicationInfo;
        applicationInfo.setPApplicationName(appName.c_str())
                        .setApiVersion(vk::ApiVersion13)
                        .setPEngineName("No Engine")
                        .setEngineVersion(vk::makeApiVersion(0, 0, 1, 0));

        uint32_t extensionCount = 0;
        const char **ppExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        extensions.insert(extensions.end(), ppExtensions, ppExtensions + extensionCount);
                        
        vk::InstanceCreateInfo createInfo;
        createInfo.setPApplicationInfo(&applicationInfo)
                    .setPEnabledExtensionNames(extensions)
                    .setPEnabledLayerNames(layers)
                    .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
        
        _instance = vk::createInstance(createInfo);
        std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
    }


    void ResourceManager::addResource(ResourceType type, std::string name, ResourceBase *pResource)
    {
        std::unordered_map<std::string, ResourceBase *> &resources = _pResources[type];
        std::unordered_map<std::string, ResourceBase *>::iterator iter = resources.find(name);
        if (iter != resources.end())
        {
#ifdef DEBUG
            std::cout << "[Warning] Resource with name " << toString(type) << "_" << name << " already exists. Cannot add resource. So the old resource deleted." << std::endl;
#endif
            delete iter->second; // Clean up the old resource to avoid memory leak
        }
        resources.insert({name, pResource});
    }

    ResourceSet &ResourceManager::resources()
    {
        return _pResources;
    }

    ResourceBase *ResourceManager::resource(ResourceType type, std::string name)
    {
        std::unordered_map<std::string, ResourceBase *> &resources = _pResources[type];
        std::unordered_map<std::string, ResourceBase *>::iterator iter = _pResources[type].find(name);
        if (iter != resources.end())
            return iter->second;
        else
            return nullptr;
    }

    void ResourceManager::remove(ResourceType type, std::string name)
    {
        if (!_pResources.count(type))
        {
#ifdef DEBUG
            std::cout << "Failed to remove a resource, because it is not exist." << std::endl;
#endif
            return ;
        }

        std::unordered_map<std::__1::string, Vkbase::ResourceBase *>::iterator iter = _pResources[type].find(name);
        if (iter == _pResources[type].end())
        {
#ifdef DEBUG
            std::cout << "Failed to remove a resource, because it is not exist." << std::endl;
#endif
            return ;
        }
        iter->second->disconnect();
        delete iter->second;
        _pResources[type].erase(iter);

        if (_pResources[type].empty())
            _pResources.erase(type);
        
#ifdef DEBUG
        std::cout << "Success to remove the resource. Type: " << toString(type) << ", Name: " << name << std::endl;
#endif
    }

    vk::Instance &ResourceManager::instance()
    {
        return _instance;
    }
}