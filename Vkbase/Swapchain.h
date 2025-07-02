#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
    class Device; // Forward declaration of Device class
    class SurfaceSupportDetails; // Forward declaration of Device class
    class Image;
    class Window;

    class Swapchain : public ResourceBase
    {
    private:
        vk::SwapchainKHR _swapchain;
        vk::SurfaceFormatKHR _format;
        vk::Extent2D _extent;
        vk::PresentModeKHR _presentMode;
        std::vector<vk::Image> _images;
        std::vector<vk::ImageView> _imageViews;
        std::vector<std::string> _imageNames;
        const Device &_device;
        const vk::SurfaceKHR &_surface;

        void init();
        void cleanup();
        void determineFormat(SurfaceSupportDetails &details);
        void determineExtent(SurfaceSupportDetails &details);
        void determinePresentMode(SurfaceSupportDetails &details);

    public:
        Swapchain(const std::string& resourceName, const std::string &deviceName, const std::string &windowName, uint32_t width, uint32_t height);
        ~Swapchain() override;

        vk::Format format() const;
        vk::Extent2D extent() const;
        const vk::SwapchainKHR &swapchain() const;
        const std::vector<vk::Image> &images() const;
        const std::vector<std::string> &imageNames() const;
        const std::vector<vk::ImageView> &imageViews() const;
    };
}