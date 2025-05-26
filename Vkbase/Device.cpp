#include "./Device.h"

namespace Vkbase
{
    Device::Device(const std::string &resourceName, vk::SurfaceKHR &surface)
        : ResourceBase(ResourceType::Device, resourceName)
    {
        pickPhysicalDevice(surface);
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
    }

    void Device::pickPhysicalDevice(vk::SurfaceKHR &surface)
    {
        std::vector<vk::PhysicalDevice> _physicalDevices = resourceManager().instance().enumeratePhysicalDevices();
        for (const auto &device : _physicalDevices)
            if (isphysicalDeviceSuitable(device, surface))
            {
                _physicalDevice = device;
                break;
            }
        if (!_physicalDevice)
            throw std::runtime_error("Failed to find a suitable GPU");
    }

    bool Device::isphysicalDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        _queueFamilyIndices = findQueueFamilies(device, surface);

        // Check if the device supports the required features
        vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();
        return _queueFamilyIndices.isComplete() && supportedFeatures.samplerAnisotropy && supportedFeatures.geometryShader && checkSwapChainSupport(device, surface);
    }

    Device::QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface)
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

    bool Device::checkSwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        if (checkDeviceExtensionSupport(device))
        {
            SurfaceSupportDetails supportDetails = querySwapChainSupport(device, surface);
            return !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
        }
        return false;
    }

    bool Device::checkDeviceExtensionSupport(vk::PhysicalDevice device)
    {
        std::unordered_set extensions(_extensions.begin(), _extensions.end());
        for (const auto &extension : device.enumerateDeviceExtensionProperties())
            extensions.erase(extension.extensionName);
        return extensions.empty();
    }

    SurfaceSupportDetails Device::querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        SurfaceSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes = device.getSurfacePresentModesKHR(surface);
        return details;
    }

    vk::Device Device::device()
    {
        return _device;
    }
    vk::PhysicalDevice Device::physicalDevice()
    {
        return _physicalDevice;
    }
    vk::Queue Device::graphicsQueue()
    {
        return _graphicsQueue;
    }
    vk::Queue Device::presentQueue()
    {
        return _presentQueue;
    }

    Device::QueueFamilyIndices Device::queueFamilyIndices()
    {
        return _queueFamilyIndices;
    }
    
    Device *Device::getSuitableDevice(vk::SurfaceKHR &surface)
    {
        return ;
    }
    
}