#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include "ResourceBase.h"

namespace Vkbase
{
    class Device; // Forward declaration of Device class
    class Swapchain : public ResourceBase
    {
    private:
        vk::SwapchainKHR _swapchain;
        vk::SurfaceFormatKHR _format;
        vk::Extent2D _extent;
        vk::PresentModeKHR _presentMode;
        std::vector<vk::Image> _images;
        std::vector<vk::ImageView> _imageViews;
        Device &_device;
        vk::SurfaceKHR _surface;

        uint32_t _imageCount;


        void init();
        void determineFormat(Device::SurfaceSupportDetails &details);
        void determineExtent(Device::SurfaceSupportDetails &details);
        void determinePresentMode(Device::SurfaceSupportDetails &details);
        void cleanup();

    public:
        Swapchain(const std::string& resourceName, const std::string &deviceName, vk::SurfaceKHR surface, uint32_t width, uint32_t height);
        ~Swapchain() override;

        vk::SwapchainKHR swapchain() const { return _swapchain; }
        vk::Extent2D extent() const { return _extent; }
        const std::vector<vk::Image>& images() const { return _images; }
        const std::vector<vk::ImageView>& imageViews() const { return _imageViews; }
    };
}