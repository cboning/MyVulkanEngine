#include "VkResourceManager.h"
#include <iostream>

#include "VkResourceBase.h"
#include "Window.h"

namespace Vkbase
{
VkResourceManager::VkResourceManager()
{
    if (!glfwInit())
    {
        std::cerr << "[Error] Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    createInstance({"VK_LAYER_KHRONOS_validation"}, {
#ifdef __APPLE__
                                                        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, "VK_MVK_macos_surface"
#endif
                                                    });
}

VkResourceManager::~VkResourceManager()
{
    while (_pResources.count(Vkbase::VkResourceType::Device))
        _pResources[Vkbase::VkResourceType::Device].begin()->second->destroy();

    _instance.destroy();
}

void VkResourceManager::createInstance(std::vector<const char *> layers, std::vector<const char *> extensions, std::string appName)
{
    vk::ApplicationInfo applicationInfo;
    applicationInfo.setPApplicationName(appName.c_str())
        .setApiVersion(vk::ApiVersion13)
        .setPEngineName("No Engine")
        .setEngineVersion(vk::makeApiVersion(0, 0, 1, 0));

    uint32_t extensionCount = 0;
    const char **ppExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    extensions.insert(extensions.end(), ppExtensions, ppExtensions + extensionCount);

    std::vector<const char *> tempLayerNames;

    std::vector<vk::LayerProperties> usableLayers = vk::enumerateInstanceLayerProperties();
    std::vector<std::string> usableLayerNames;
    for (const vk::LayerProperties &layer : usableLayers)
        usableLayerNames.push_back(layer.layerName);

    for (const char *layerName : layers)
        if (std::find(usableLayerNames.begin(), usableLayerNames.end(), layerName) != usableLayerNames.end())
            tempLayerNames.push_back(layerName);

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&applicationInfo)
        .setPEnabledExtensionNames(extensions)
        .setPEnabledLayerNames(tempLayerNames)
        .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);

    _instance = vk::createInstance(createInfo);
    if (!_instance)
        throw std::runtime_error("[ERROR] Failed to create vulkan instance.");
    std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
}

void VkResourceManager::addResource(VkResourceBase *pResource)
{
    std::unordered_map<std::string, VkResourceBase *> &resources = _pResources[pResource->type()];
    std::unordered_map<std::string, VkResourceBase *>::iterator iter = resources.find(pResource->name());
    if (iter != resources.end())
    {
        if (iter->second == pResource)
            return;
#ifdef DEBUG
        std::cout << "[Warning] Resource with Type: " << toString(pResource->type()) << ", Name: " << pResource->name()
                  << " already exists. Cannot add resource. So the old resource is "
                     "destroyed."
                  << std::endl;
#endif
        iter->second->destroy();

        _pResources[pResource->type()].insert({pResource->name(), pResource});
    }
    else
        resources.insert({pResource->name(), pResource});
}

const VkResourceSet &VkResourceManager::resources() const { return _pResources; }

VkResourceBase *VkResourceManager::resource(VkResourceType type, const std::string &name) const
{
    Vkbase::VkResourceSet::const_iterator typeIter = _pResources.find(type);
    if (typeIter == _pResources.end())
        return nullptr;
    const std::unordered_map<std::string, VkResourceBase *> &resources = typeIter->second;
    const std::unordered_map<std::string, VkResourceBase *>::const_iterator iter = resources.find(name);
    if (iter != resources.end())
        return iter->second;
    else
        return nullptr;
}

void VkResourceManager::remove(VkResourceType type, const std::string &name)
{
    if (!_pResources.count(type))
    {
#ifdef DEBUG
        std::cout << "[Warning] Failed to remove a resource, because it is not "
                     "exist. Type: "
                  << toString(type) << ", Name: " << name << std::endl;
#endif
        return;
    }

    std::unordered_map<std::string, Vkbase::VkResourceBase *>::iterator iter = _pResources[type].find(name);
    if (iter == _pResources[type].end())
    {
#ifdef DEBUG
        std::cout << "[Warning] Failed to remove a resource, because it is not exist. Type: " << toString(type) << ", Name: " << name << std::endl;
#endif
        return;
    }
    Vkbase::VkResourceBase *pBase = iter->second;
    _pResources[type].erase(iter);
    pBase->preDestroy();
    if (!pBase->killingSelf())
        delete pBase;

    if (_pResources[type].empty())
        _pResources.erase(type);
}

VkResourceManager &VkResourceManager::instance()
{
    static VkResourceManager instance;
    return instance;
}

const vk::Instance &VkResourceManager::vkInstance() const { return _instance; }
} // namespace Vkbase