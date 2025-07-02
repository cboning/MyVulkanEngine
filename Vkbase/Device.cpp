#include "Device.h"
#include "CommandPool.h"

namespace Vkbase
{
    Device::Device(const std::string &resourceName, const vk::SurfaceKHR &surface)
        : ResourceBase(ResourceType::Device, resourceName)
    {
        _physicalDevice = pickPhysicalDevice(surface);
        _queueFamilyIndices = findQueueFamilies(_physicalDevice, surface);
        createLogicalDevice();
    }

    Device::~Device()
    {
        _device.waitIdle();
        _device.destroy();
    }

    void Device::createLogicalDevice()
    {
        vk::PhysicalDeviceFeatures deviceFeatures;
        deviceFeatures.setSamplerAnisotropy(vk::Bool32(true));
        // .setGeometryShader(vk::Bool32(true));

        vk::DeviceQueueCreateInfo queueCreateInfos[3];
        uint32_t queueCount = 0;
        float queuePriority = 1.0f;
        for (int i = 0; i < 3; i++)
            queueCreateInfos[i].setQueueCount(1).setPQueuePriorities(&queuePriority);

        queueCreateInfos[queueCount++].setQueueFamilyIndex(_queueFamilyIndices.graphicsFamilyIndex);
        if (_queueFamilyIndices.computeFamilyIndex != _queueFamilyIndices.graphicsFamilyIndex)
            queueCreateInfos[queueCount++].setQueueFamilyIndex(_queueFamilyIndices.computeFamilyIndex);
        if (_queueFamilyIndices.presentFamilyIndex != _queueFamilyIndices.graphicsFamilyIndex && _queueFamilyIndices.presentFamilyIndex != _queueFamilyIndices.computeFamilyIndex)
            queueCreateInfos[queueCount++].setQueueFamilyIndex(_queueFamilyIndices.presentFamilyIndex);

        vk::DeviceCreateInfo createInfo;
        createInfo.setQueueCreateInfoCount(queueCount)
            .setPQueueCreateInfos(queueCreateInfos)
            .setPEnabledFeatures(&deviceFeatures)
            .setPEnabledExtensionNames(_extensions)
            .setPEnabledLayerNames(_layers);
        
        if (_physicalDevice.createDevice(&createInfo, nullptr, &_device) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create logical device");

        _graphicsQueue = _device.getQueue(_queueFamilyIndices.graphicsFamilyIndex, 0);
        _presentQueue = _device.getQueue(_queueFamilyIndices.presentFamilyIndex, 0);
        _computeQueue = _device.getQueue(_queueFamilyIndices.computeFamilyIndex, 0);
    }

    vk::PhysicalDevice Device::pickPhysicalDevice(const vk::SurfaceKHR &surface)
    {
        std::vector<vk::PhysicalDevice> physicalDevices = resourceManager().instance().enumeratePhysicalDevices();
        for (const auto &device : physicalDevices)
            if (isPhysicalDeviceSuitable(device, surface))
                return device;
        throw std::runtime_error("Failed to find a suitable GPU");
    }

    bool Device::isPhysicalDeviceSuitable(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    {
        Device::QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device, surface);
        // Check if the device supports the required features
        vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();
        return queueFamilyIndices.isComplete() && supportedFeatures.samplerAnisotropy && checkSwapChainSupport(device, surface);
    }

    Device::QueueFamilyIndices Device::findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    {
        QueueFamilyIndices indices;
        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilies.size(); i++)
        {
            vk::Flags supportGraphics = queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics;
            vk::Flags supportCompute = queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute;
            vk::Bool32 supportPresent = vk::False;

            if (device.getSurfaceSupportKHR(i, surface, &supportPresent) != vk::Result::eSuccess)
                throw std::runtime_error("Failed to get surface support");

            if (supportGraphics && supportCompute && supportPresent)
            {
                indices.graphicsFamilyIndex = indices.computeFamilyIndex = indices.presentFamilyIndex = static_cast<int>(i);
                break;
            }

            if (supportGraphics)
                indices.graphicsFamilyIndex = static_cast<int>(i);
            if (supportCompute)
                indices.computeFamilyIndex = static_cast<int>(i);
            if (supportPresent)
                indices.presentFamilyIndex = static_cast<int>(i);
        }
        return indices;
    }

    bool Device::checkSwapChainSupport(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    {
        if (checkDeviceExtensionSupport(device))
        {
            SurfaceSupportDetails supportDetails = querySwapChainSupport(device, surface);
            return !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
        }
        return false;
    }

    bool Device::checkDeviceExtensionSupport(const vk::PhysicalDevice &device)
    {
        std::unordered_set<std::string> extensions(_extensions.begin(), _extensions.end());
        auto extensionst = device.enumerateDeviceExtensionProperties();
        for (const auto &extension : extensionst)
        {
            std::string extName(extension.extensionName.data());
            extensions.erase(extName);
        }
        return extensions.empty();
    }

    SurfaceSupportDetails Device::querySwapChainSupport(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    {
        SurfaceSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes = device.getSurfacePresentModesKHR(surface);
        return details;
    }

    const vk::Device &Device::device() const
    {
        return _device;
    }
    const vk::PhysicalDevice &Device::physicalDevice() const
    {
        return _physicalDevice;
    }
    const vk::Queue &Device::graphicsQueue() const
    {
        return _graphicsQueue;
    }
    const vk::Queue &Device::presentQueue() const
    {
        return _presentQueue;
    }

    const vk::Queue &Device::computeQueue() const
    {
        return _computeQueue;
    }

    const Device::QueueFamilyIndices &Device::queueFamilyIndices() const
    {
        return _queueFamilyIndices;
    }
    
    Device *Device::getSuitableDevice(const vk::SurfaceKHR &surface)
    {
        if (resourceManager().resources().count(ResourceType::Device))
            for (const auto pDevice : resourceManager().resources().at(ResourceType::Device))
            {
                Device &targetDevice = *dynamic_cast<Device *>(pDevice.second);
                if (isPhysicalDeviceSuitable(targetDevice.physicalDevice(), surface) && targetDevice.queueFamilyIndices() == findQueueFamilies(targetDevice.physicalDevice(), surface))
                    return &targetDevice;
            }
        return new Device("", surface);
    }
}