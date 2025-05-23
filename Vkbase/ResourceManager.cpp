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


    void ResourceManager::addResource(std::string name, ResourceBase *pResource)
    {
        Vkbase::ResourceSet::iterator iter = _resources.find(name);
        if (iter != _resources.end())
        {
            std::cout << "[Warning] Resource with name " << name << " already exists. Cannot add resource. So the old resource deleted." << std::endl;
            delete iter->second; // Clean up the old resource to avoid memory leak
        }
        _resources.insert({name, pResource});
    }

    ResourceSet &ResourceManager::resources()
    {
        return _resources;
    }

    void ResourceManager::remove(std::string name)
    {
        delete _resources.extract(name).mapped();
    }

    vk::Instance &ResourceManager::instance()
    {
        return _instance;
    }
}