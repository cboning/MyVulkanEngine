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
            std::cerr << "[Error] Failed to initialize GLFW" << std::endl;
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
        if (!_instance)
            throw std::runtime_error("[ERROR] Failed to create vulkan instance.");
        std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
    }

    void ResourceManager::addResource(ResourceBase *pResource)
    {
        std::unordered_map<std::string, ResourceBase *> &resources = _pResources[pResource->type()];
        std::unordered_map<std::string, ResourceBase *>::iterator iter = resources.find(pResource->name());
        if (iter != resources.end())
        {
            if (iter->second == pResource)
                return ;
#ifdef DEBUG
            std::cout << "[Warning] Resource with Type: " << toString(pResource->type()) << ", Name: " << pResource->name() << " already exists. Cannot add resource. So the old resource is destroyed." << std::endl;
#endif
            iter->second->destroy();
            
            _pResources[pResource->type()].insert({pResource->name(), pResource});
        }
        else
            resources.insert({pResource->name(), pResource});
    }

    const ResourceSet &ResourceManager::resources() const
    {
        return _pResources;
    }

    ResourceBase *ResourceManager::resource(ResourceType type, std::string name) const
    {
        Vkbase::ResourceSet::const_iterator typeIter = _pResources.find(type);
        if (typeIter == _pResources.end())
            return nullptr;
        const std::unordered_map<std::string, ResourceBase *> &resources = typeIter->second;
        const std::unordered_map<std::string, ResourceBase *>::const_iterator iter = resources.find(name);
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
            std::cout << "[Warning] Failed to remove a resource, because it is not exist. Type: " << toString(type) << ", Name: " << name << std::endl;
#endif
            return ;
        }

        std::unordered_map<std::string, Vkbase::ResourceBase *>::iterator iter = _pResources[type].find(name);
        if (iter == _pResources[type].end())
        {
#ifdef DEBUG
            std::cout << "[Warning] Failed to remove a resource, because it is not exist. Type: " << toString(type) << ", Name: " << name << std::endl;
#endif
            return ;
        }
        iter->second->preDestroy();
        delete iter->second;
        _pResources[type].erase(iter);

        if (_pResources[type].empty())
            _pResources.erase(type);
    }

    void ResourceManager::renameResource(const ResourceBase *pResource, const std::string &newName)
    {
        ResourceType type = pResource->type();
        Vkbase::ResourceSet::const_iterator typeIter = _pResources.find(type);
        if (typeIter == _pResources.end())
            std::cout << "[ERROR] Failed to rename the resouce." << std::endl;
            return ;
        const std::unordered_map<std::string, ResourceBase *> &resources = typeIter->second;
        const std::unordered_map<std::string, ResourceBase *>::const_iterator iter = resources.find(pResource->name());
        if (iter == resources.end())
            std::cout << "[ERROR] Failed to rename the resouce." << std::endl;
            return ;

        const ResourceBase *pTempResource = resource(type, newName);
        if (pTempResource)
            pTempResource->destroy();

        iter->second->rename(newName);
        _pResources[type].erase(iter);
        _pResources[type].insert({newName, iter->second});
    }

    const vk::Instance &ResourceManager::instance() const
    {
        return _instance;
    }
}