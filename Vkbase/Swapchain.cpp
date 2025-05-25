#include "Swapchain.h"
#include "Device.h"
#include "cmath"


namespace Vkbase
{
    Swapchain::Swapchain(const std::string& resourceName, const std::string &deviceName, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
        : ResourceBase(resourceName, ResourceType::Swapchain), _device(*dynamic_cast<Device *>(resourceManager().resource(deviceName))), _surface(surface)
    {
        _extent.setWidth(width).setHeight(height);
        Device::SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);
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
        Device::SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);
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
        
        _swapchain = _device.device().createSwapchainKHR(createInfo);

        // Get the images from the swapchain
        _images = _device.device().getSwapchainImagesKHR(_swapchain);

        // Create image views for each image
        for (const auto& image : _images) {
            vk::ImageViewCreateInfo viewInfo;
            viewInfo.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(_imageFormat)
                .setComponents({ vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity })
                .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

            _imageViews.push_back(_device.createImageView(viewInfo));
        }
    }

    void Swapchain::determineExtent(Device::SurfaceSupportDetails &details)
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

    void Swapchain::determineFormat(Device::SurfaceSupportDetails &details)
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


    void Swapchain::determinePresentMode(Device::SurfaceSupportDetails &details)
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
    vk::SwapchainKHR Swapchain::swapchain() const
    {
        return _swapchain;
    }
    vk::Format Swapchain::imageFormat() const
    {
        return _imageFormat;
    }
    vk::Extent2D Swapchain::extent() const
    {
        return _extent;
    }
    const std::vector<vk::Image>& Swapchain::images() const
    {
        return _images;
    }
    const std::vector<vk::ImageView>& Swapchain::imageViews() const
    {
        return _imageViews;
    }
    void Swapchain::setDevice(vk::Device device)
    {
        _device = device;
    }
    void Swapchain::setSurface(vk::SurfaceKHR surface)
    {
        _surface = surface;
    }
    void Swapchain::setWidth(uint32_t width)
    {
        _width = width;
    }
    void Swapchain::setHeight(uint32_t height)
    {
        _height = height;
    }
    void Swapchain::setImageFormat(vk::Format imageFormat)
    {
        _imageFormat = imageFormat;
    }
    void Swapchain::setExtent(vk::Extent2D extent)
    {
        _extent = extent;
    }
}