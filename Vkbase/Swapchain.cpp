#include "Swapchain.h"
#include "Device.h"
#include "Image.h"
#include "Window.h"
#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Vkbase
{
Swapchain::Swapchain(const std::string &resourceName, const std::string &deviceName, const std::string &windowName)
    : GpuResourceBase(ResourceType::Swapchain, resourceName, *dynamic_cast<Device *>(resourceManager().resource(ResourceType::Device, deviceName))),
      _window(*dynamic_cast<Window *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Window, windowName)))), _surface(_window.surface())
{
    try
    {
        _extent.setWidth(_window.width()).setHeight(_window.height());

        SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);

        determineExtent(supportDetails);
        determineFormat(supportDetails);
        determinePresentMode(supportDetails);
        init();
    }
    catch (const std::exception &e)
    {
        std::stringstream ss;
        ss << "Failed to create Swapchain '" << resourceName << "': " << e.what();
        cleanup();
        throw std::runtime_error(ss.str());
    }
    catch (...)
    {
        cleanup();
        throw std::runtime_error("Unknown error occurred during Swapchain creation");
    }
}

Swapchain::~Swapchain()
{
    auto device = _device.device();
    auto swapchain = _swapchain;
    auto imageViews = _imageViews;

    _onDelayDestroy = [device, swapchain, imageViews]() mutable
    {
        for (const auto &imageView : imageViews)
            if (imageView)
                device.destroy(imageView);
        imageViews.clear();

        if (swapchain)
            device.destroy(swapchain);
    };
}

Swapchain *Swapchain::recreate()
{
    try
    {
        Window &window = _window;
        const std::string resourceName = name();
        const std::string deviceName = _device.name();
        const std::string windowName = window.name();

        window.setLock();
        destroy();
        Swapchain *pNewSwapchain = new Swapchain(resourceName, deviceName, windowName);
        window.setUnlock();

        return pNewSwapchain;
    }
    catch (const std::exception &e)
    {
        _window.setUnlock();
        std::stringstream ss;
        ss << "Failed to recreate Swapchain '" << name() << "': " << e.what();
        throw std::runtime_error(ss.str());
    }
    catch (...)
    {
        _window.setUnlock();
        throw std::runtime_error("Unknown error occurred during Swapchain recreation");
    }
}

void Swapchain::init()
{
    vk::SwapchainKHR tempSwapchain = VK_NULL_HANDLE;
    std::vector<vk::ImageView> tempImageViews;

    try
    {
        SurfaceSupportDetails supportDetails = _device.querySwapChainSupport(_device.physicalDevice(), _surface);
        uint32_t desiredImageCount = 5;
        if (supportDetails.capabilities.maxImageCount)
            desiredImageCount = std::min(desiredImageCount, supportDetails.capabilities.maxImageCount);

        desiredImageCount = std::max(desiredImageCount, supportDetails.capabilities.minImageCount + 1);

        if (supportDetails.formats.empty())
            throw std::runtime_error("No surface formats supported");

        if (supportDetails.presentModes.empty())
            throw std::runtime_error("No present modes supported");

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
            .setPresentMode(_presentMode)
            .setClipped(vk::True);

        Device::QueueFamilyIndices deviceQueueFamilyIndice = _device.queueFamilyIndices();
        uint32_t queueFamilyIndice[3];
        uint32_t queueFamilyIndiceCount = 0;

        queueFamilyIndice[queueFamilyIndiceCount++] = deviceQueueFamilyIndice.graphicsFamilyIndex;
        if (deviceQueueFamilyIndice.computeFamilyIndex != deviceQueueFamilyIndice.graphicsFamilyIndex)
            queueFamilyIndice[queueFamilyIndiceCount++] = deviceQueueFamilyIndice.computeFamilyIndex;
        if (deviceQueueFamilyIndice.presentFamilyIndex != deviceQueueFamilyIndice.graphicsFamilyIndex &&
            deviceQueueFamilyIndice.presentFamilyIndex != deviceQueueFamilyIndice.computeFamilyIndex)
            queueFamilyIndice[queueFamilyIndiceCount++] = deviceQueueFamilyIndice.presentFamilyIndex;

        if (queueFamilyIndiceCount == 1)
            createInfo.setQueueFamilyIndices(queueFamilyIndice[0]).setQueueFamilyIndexCount(1).setImageSharingMode(vk::SharingMode::eExclusive);
        else
            createInfo.setPQueueFamilyIndices(queueFamilyIndice)
                .setQueueFamilyIndexCount(queueFamilyIndiceCount)
                .setImageSharingMode(vk::SharingMode::eConcurrent);

        tempSwapchain = _device.device().createSwapchainKHR(createInfo);

        _images = _device.device().getSwapchainImagesKHR(tempSwapchain);
        std::cout << "Swapchain image count: " << _images.size() << std::endl;

        if (_images.empty())
        {
            throw std::runtime_error("Failed to get swapchain images");
        }

        for (const auto &image : _images)
        {
            vk::ImageViewCreateInfo viewInfo;
            vk::ComponentMapping components;
            vk::ImageSubresourceRange subresourceRange;
            subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor).setBaseArrayLayer(0).setLayerCount(1).setBaseMipLevel(0).setLevelCount(1);

            viewInfo.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(_format.format)
                .setComponents(components)
                .setSubresourceRange(subresourceRange);

            try
            {
                tempImageViews.push_back(_device.device().createImageView(viewInfo));
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error(std::string("Failed to create image view: ") + e.what());
            }
        }

        _swapchain = tempSwapchain;
        _imageViews = std::move(tempImageViews);
        _imageNames = Image::getImagesWithSwapchain(*this);
    }
    catch (const vk::SystemError &e)
    {

        cleanupTemporary(tempSwapchain, tempImageViews);
        std::stringstream ss;
        ss << "Vulkan system error during swapchain initialization: " << e.what();
        throw std::runtime_error(ss.str());
    }
    catch (const std::exception &e)
    {

        cleanupTemporary(tempSwapchain, tempImageViews);
        std::stringstream ss;
        ss << "Error during swapchain initialization: " << e.what();
        throw std::runtime_error(ss.str());
    }
    catch (...)
    {

        cleanupTemporary(tempSwapchain, tempImageViews);
        throw std::runtime_error("Unknown error during swapchain initialization");
    }
}

void Swapchain::cleanupTemporary(vk::SwapchainKHR tempSwapchain, const std::vector<vk::ImageView> &tempImageViews)
{
    try
    {

        for (const auto &imageView : tempImageViews)
        {
            if (imageView)
            {
                _device.device().destroy(imageView);
            }
        }

        if (tempSwapchain)
        {
            _device.device().destroy(tempSwapchain);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during temporary resource cleanup: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error during temporary resource cleanup" << std::endl;
    }
}

void Swapchain::determineExtent(SurfaceSupportDetails &details)
{
    try
    {
        vk::SurfaceCapabilitiesKHR &capabilities = details.capabilities;
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            _extent = capabilities.currentExtent;
            return;
        }

        _extent.width = std::max(std::min(capabilities.maxImageExtent.width, _extent.width), capabilities.minImageExtent.width);
        _extent.height = std::max(std::min(capabilities.maxImageExtent.height, _extent.height), capabilities.minImageExtent.height);

        if (_extent.width == 0 || _extent.height == 0)
        {
            throw std::runtime_error("Invalid swapchain extent: width or height is zero");
        }
    }
    catch (const std::exception &e)
    {
        std::stringstream ss;
        ss << "Failed to determine swapchain extent: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

void Swapchain::determineFormat(SurfaceSupportDetails &details)
{
    try
    {
        std::vector<vk::SurfaceFormatKHR> &formats = details.formats;
        if (formats.empty())
        {
            throw std::runtime_error("No available surface formats");
        }

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
    catch (const std::exception &e)
    {
        std::stringstream ss;
        ss << "Failed to determine swapchain format: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

void Swapchain::determinePresentMode(SurfaceSupportDetails &details)
{
    try
    {
        std::vector<vk::PresentModeKHR> &presentModes = details.presentModes;
        if (presentModes.empty())
        {
            throw std::runtime_error("No available present modes");
        }

        vk::PresentModeKHR desirableMode = vk::PresentModeKHR::eFifo;
        for (const vk::PresentModeKHR &presentMode : presentModes)
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
    catch (const std::exception &e)
    {
        std::stringstream ss;
        ss << "Failed to determine present mode: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

void Swapchain::cleanup()
{
    try
    {
        if (_cleaned)
            return;
        _cleaned = true;

        for (const auto &imageView : _imageViews)
            if (imageView)
                _device.device().destroy(imageView);
        _imageViews.clear();

        if (_swapchain)
        {
            _device.device().destroy(_swapchain);
            _swapchain = VK_NULL_HANDLE;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during Swapchain cleanup: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error during Swapchain cleanup" << std::endl;
    }
}

const vk::SwapchainKHR &Swapchain::swapchain() const
{
    if (!_swapchain)
        throw std::runtime_error("Swapchain is not initialized");
    return _swapchain;
}

vk::Format Swapchain::format() const { return _format.format; }

vk::Extent2D Swapchain::extent() const
{
    if (_extent.width == 0 || _extent.height == 0)
    {
        throw std::runtime_error("Swapchain extent is not valid");
    }
    return _extent;
}

const std::vector<vk::Image> &Swapchain::images() const
{
    if (_images.empty())
    {
        throw std::runtime_error("No swapchain images available");
    }
    return _images;
}

const std::vector<vk::ImageView> &Swapchain::imageViews() const
{
    if (_imageViews.empty())
    {
        throw std::runtime_error("No swapchain image views available");
    }
    return _imageViews;
}

Device &Swapchain::device() { return _device; }

const std::vector<std::string> &Swapchain::imageNames() const { return _imageNames; }

} // namespace Vkbase