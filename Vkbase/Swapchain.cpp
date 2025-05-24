#include "Swapchain.h"
#include "Device.h"


namespace Vkbase
{
    Swapchain::Swapchain(const std::string& resourceName, const std::string deviceName, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
        : ResourceBase(resourceName, ResourceType::Swapchain), _device(*dynamic_cast<Device *>(resourceManager().resource(deviceName))), _surface(surface)
    {
        _extent.setWidth(width).setHeight(height);
        init();
    }

    Swapchain::~Swapchain()
    {
        cleanup();
    }

    void Swapchain::init()
    {
        Device::SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);
        // Get the surface capabilities

        // Choose the format and extent
        _imageFormat = vk::Format::eB8G8R8A8Srgb;

        // Create the swapchain
        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setSurface(_surface)
            .setMinImageCount(supportDetails.capabilities.minImageCount)
            .setImageFormat(_imageFormat)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(_extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setPreTransform(surfaceCapabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setClipped(VK_TRUE);

        _swapchain = _device.createSwapchainKHR(createInfo);

        // Get the images from the swapchain
        _images = _device.getSwapchainImagesKHR(_swapchain);

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
    void Swapchain::cleanup()
    {
        for (const auto& imageView : _imageViews) {
            _device.destroyImageView(imageView);
        }
        _device.destroySwapchainKHR(_swapchain);
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