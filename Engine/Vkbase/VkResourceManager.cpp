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
                                                        "VK_MVK_macos_surface"
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
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName(appName.c_str()).setApiVersion(VK_API_VERSION_1_3).setPEngineName("No Engine").setEngineVersion(vk::makeApiVersion(0, 0, 1, 0));

    uint32_t glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    extensions.insert(extensions.end(), glfwExts, glfwExts + glfwExtCount);

#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    std::vector<vk::ExtensionProperties> availableExts = vk::enumerateInstanceExtensionProperties();
    std::vector<std::string> availableExtNames;
    for (auto &e : availableExts)
        availableExtNames.push_back(e.extensionName);

    std::vector<const char *> enabledExts;
    for (const char *ext : extensions)
    {
        if (std::find(availableExtNames.begin(), availableExtNames.end(), ext) != availableExtNames.end())
            enabledExts.push_back(ext);
#ifdef DEBUG
        else
            std::cout << "[Warning] Extension not found: " << ext << std::endl;
#endif
    }

    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
    std::vector<std::string> availableLayerNames;
    for (auto &l : availableLayers)
        availableLayerNames.push_back(l.layerName);

    std::vector<const char *> enabledLayers;
    for (const char *layer : layers)
    {
        if (std::find(availableLayerNames.begin(), availableLayerNames.end(), layer) != availableLayerNames.end())
            enabledLayers.push_back(layer);
#ifdef DEBUG
        else
            std::cout << "[Warning] Layer not found: " << layer << std::endl;
#endif
    }

    vk::InstanceCreateInfo createInfo;
#ifdef __APPLE__
    createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif
    createInfo.setPApplicationInfo(&appInfo).setPEnabledExtensionNames(enabledExts).setPEnabledLayerNames(enabledLayers);

    try
    {
        _instance = vk::createInstance(createInfo);
    }
    catch (const vk::ExtensionNotPresentError &e)
    {
        std::cerr << "[Error] Vulkan extension missing: " << e.what() << std::endl;
        throw;
    }

    if (!_instance)
        throw std::runtime_error("[ERROR] Failed to create Vulkan instance.");

#ifdef DEBUG
    std::cout << "[Info] Vulkan instance created with extensions:" << std::endl;
    for (auto *ext : enabledExts)
        std::cout << "    " << ext << std::endl;
#endif

    auto devices = _instance.enumeratePhysicalDevices();
    if (devices.empty())
        throw std::runtime_error("[Error] No Vulkan-compatible GPU found.");
}

void VkResourceManager::addResource(VkResourceBase *pResource)
{
    std::unique_lock controlMutex(_controlMutex);
    std::unordered_map<std::string, VkResourceBase *> &resources = _pResources[pResource->type()];
    std::unordered_map<std::string, VkResourceBase *>::iterator iter = resources.find(pResource->name());
    VkResourceBase *pOldResource = nullptr;
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
        pOldResource = iter->second;
        resources.erase(iter);
    }
    _pResources[pResource->type()].emplace(pResource->name(), pResource);

    controlMutex.unlock();
    destroy(pOldResource);
}

const VkResourceSet &VkResourceManager::resources() const { return _pResources; }

void VkResourceManager::destroy(VkResourceBase *pResource)
{
    if (!pResource)
        return;
    pResource->preDestroy();
    if (!pResource->killingSelf())
        delete pResource;
}

VkResourceManagerHolder::WeakReference VkResourceManager::resource(VkResourceType type, const std::string &name) const
{
    Vkbase::VkResourceSet::const_iterator typeIter = _pResources.find(type);
    if (typeIter == _pResources.end())
        return VkResourceManagerHolder::WeakReference();
    const std::unordered_map<std::string, VkResourceBase *> &resources = typeIter->second;
    const std::unordered_map<std::string, VkResourceBase *>::const_iterator iter = resources.find(name);
    if (iter != resources.end())
        return iter->second->weakReference();
    else
        return VkResourceManagerHolder::WeakReference();
}

void VkResourceManager::remove(VkResourceType type, const std::string &name)
{
    std::unique_lock lock(_controlMutex);
    if (!_pResources.count(type))
    {
#ifdef DEBUG
        std::cout << "[Warning] Failed to remove a resource, because it is not "
                     "exist. Type: "
                  << toString(type) << ", Name: " << name << std::endl;
#endif
        return;
    }

    std::unordered_map<std::string, VkResourceBase *>::iterator iter = _pResources[type].find(name);
    if (iter == _pResources[type].end())
    {
#ifdef DEBUG
        std::cout << "[Warning] Failed to remove a resource, because it is not exist. Type: " << toString(type) << ", Name: " << name << std::endl;
#endif
        return;
    }
    VkResourceBase *pResource = iter->second;
    _pResources[type].erase(iter);

    if (_pResources[type].empty())
        _pResources.erase(type);
    lock.unlock();

    destroy(pResource);
}

VkResourceManager &VkResourceManager::instance()
{
    if (!_pInstance)
        _pInstance = new VkResourceManager();

    return *_pInstance;
}

void VkResourceManager::shutDown()
{
    if (_pInstance)
    {
        delete _pInstance;
        _pInstance = nullptr;
    }
}

const vk::Instance &VkResourceManager::vkInstance() const { return _instance; }
} // namespace Vkbase