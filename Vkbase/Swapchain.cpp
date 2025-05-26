#include "Swapchain.h"
#include "Device.h"
#include "cmath"
#include <array>


namespace Vkbase
{
    Swapchain::Swapchain(const std::string& resourceName, const std::string &deviceName, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
        : ResourceBase(ResourceType::Swapchain, resourceName), _device(*dynamic_cast<Device *>(resourceManager().resource(ResourceType::Device, deviceName))), _surface(surface)
    {
        _extent.setWidth(width).setHeight(height);
        SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);
        _imageCount = _device.querySwapChainSupport(_device.physicalDevice(), _surface).capabilities.minImageCount + 1;

        // Determine some properties from surface details
        determineExtent(supportDetails);
        determineFormat(supportDetails);
        determinePresentMode(supportDetails);
        init();
    }

    Swapchain::~Swapchain()
    {
        cleanup();
    }

    void Swapchain::init()
    {
        SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);
        uint32_t desiredImageCount = 5;
        if (supportDetails.capabilities.maxImageCount)
            desiredImageCount = std::min(desiredImageCount, supportDetails.capabilities.maxImageCount);
        
        desiredImageCount = std::max(desiredImageCount, supportDetails.capabilities.minImageCount + 1);

        // Create the swapchain
        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setSurface(_surface)
            .setMinImageCount(desiredImageCount)
            .setImageFormat(_format.format)
            .setImageColorSpace(_format.colorSpace)
            .setImageExtent(_extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setPreTransform(supportDetails.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setClipped(vk::True);

        Device::QueueFamilyIndices deviceQueueFamilyIndice = _device.queueFamilyIndices();
        uint32_t queueFamilyIndice[3];
        uint32_t queueFamilyIndiceCount = 0;
        
        queueFamilyIndice[queueFamilyIndiceCount++] = deviceQueueFamilyIndice.graphicsFamilyIndex;
        if (deviceQueueFamilyIndice.computeFamilyIndex != deviceQueueFamilyIndice.graphicsFamilyIndex)
            queueFamilyIndice[queueFamilyIndiceCount++] = deviceQueueFamilyIndice.computeFamilyIndex;
        if (deviceQueueFamilyIndice.presentFamilyIndex != deviceQueueFamilyIndice.graphicsFamilyIndex && deviceQueueFamilyIndice.presentFamilyIndex != deviceQueueFamilyIndice.computeFamilyIndex)
            queueFamilyIndice[queueFamilyIndiceCount++] = deviceQueueFamilyIndice.presentFamilyIndex;

        if (queueFamilyIndiceCount == 1)
            createInfo.setQueueFamilyIndices(queueFamilyIndice[0])
                .setQueueFamilyIndexCount(1)
                .setImageSharingMode(vk::SharingMode::eExclusive);
        else
            createInfo.setPQueueFamilyIndices(queueFamilyIndice)
                .setQueueFamilyIndexCount(queueFamilyIndiceCount)
                .setImageSharingMode(vk::SharingMode::eConcurrent);
        
        _swapchain = _device.device().createSwapchainKHR(createInfo);

        // Get the images from the swapchain
        _images = _device.device().getSwapchainImagesKHR(_swapchain);

        // Create image views for each image
        for (const auto& image : _images) {
            vk::ImageViewCreateInfo viewInfo;
            vk::ComponentMapping components;
            vk::ImageSubresourceRange subresourceRange;
            subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setBaseMipLevel(0)
                .setLevelCount(1);

            viewInfo.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(_format.format)
                .setComponents(components)
                .setSubresourceRange(subresourceRange);

            _imageViews.push_back(_device.device().createImageView(viewInfo));
        }
    }

    void Swapchain::determineExtent(SurfaceSupportDetails &details)
    {
        vk::SurfaceCapabilitiesKHR &capabilities = details.capabilities;
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            _extent = capabilities.currentExtent;
            return ;
        }
        
        _extent.width = std::max(std::min(capabilities.maxImageExtent.width, _extent.width), capabilities.minImageExtent.width);
        _extent.height = std::max(std::min(capabilities.maxImageExtent.height, _extent.height), capabilities.minImageExtent.height);
    }

    void Swapchain::determineFormat(SurfaceSupportDetails &details)
    {
        std::vector<vk::SurfaceFormatKHR> &formats = details.formats;
        if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
        {
            _format = {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
            return;
        }
        for (const vk::SurfaceFormatKHR &format : formats)
            if (format.format == vk::Format::eR8G8B8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                _format = format;
                return;
            }
        _format = formats[0];
    }


    void Swapchain::determinePresentMode(SurfaceSupportDetails &details)
    {
        vk::PresentModeKHR desirableMode = vk::PresentModeKHR::eFifo;
        for (const vk::PresentModeKHR &presentMode : details.presentModes)
        {
            if (presentMode == vk::PresentModeKHR::eMailbox)
            {
                _presentMode = presentMode;
                return;
            }
            else if (presentMode == vk::PresentModeKHR::eImmediate)
                desirableMode = presentMode;
        }
        _presentMode = desirableMode;
    }

    void Swapchain::cleanup()
    {
        for (const auto& imageView : _imageViews) {
            _device.device().destroyImageView(imageView);
        }
        _device.device().destroySwapchainKHR(_swapchain);
    }

    vk::SwapchainKHR &Swapchain::swapchain()
    {
        return _swapchain;
    }

    vk::Format Swapchain::format()
    {
        return _format.format;
    }
    
    vk::Extent2D Swapchain::extent()
    {
        return _extent;
    }
    const std::vector<vk::ImageView>& Swapchain::imageViews()
    {
        return _imageViews;
    }
}