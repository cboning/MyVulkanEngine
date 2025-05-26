#include "ResourceManager.h"
#include "ResourceBase.h"
#include "Window.h"
#include <iostream>

namespace Vkbase
{
    ResourceManager::ResourceManager()
    {
        createInstance();
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
        std::vector<const char *> pGLFWRequiredExtensions(ppExtensions, ppExtensions + extensionCount);
        extensions.insert(extensions.end(), pGLFWRequiredExtensions.begin(), pGLFWRequiredExtensions.end());
                        
        vk::InstanceCreateInfo createInfo;
        createInfo.setPApplicationInfo(&applicationInfo)
                    .setPEnabledExtensionNames(extensions)
                    .setPEnabledLayerNames(layers)
                    .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
        
        _instance = vk::createInstance(createInfo);
    }


    void ResourceManager::addResource(ResourceType type, std::string name, ResourceBase *pResource)
    {
        std::unordered_map<std::string, ResourceBase *> &resources = _pResources[type];
        std::unordered_map<std::string, ResourceBase *>::iterator iter = resources.find(name);
        if (iter != resources.end())
        {
            std::cout << "[Warning] Resource with name " << toString(type) << "_" << name << " already exists. Cannot add resource. So the old resource deleted." << std::endl;
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
        delete _pResources[type].extract(name).mapped();
    }

    vk::Instance &ResourceManager::instance()
    {
        return _instance;
    }
}