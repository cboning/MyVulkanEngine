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
        vk::Format _imageFormat;
        vk::Extent2D _extent;
        std::vector<vk::Image> _images;
        std::vector<vk::ImageView> _imageViews;
        Device &_device;
        vk::SurfaceKHR _surface;


        void init();
        void cleanup();

    public:
        Swapchain(const std::string& resourceName, const std::string deviceName, vk::SurfaceKHR surface, uint32_t width, uint32_t height);
        ~Swapchain() override;

        vk::SwapchainKHR swapchain() const { return _swapchain; }
        vk::Format imageFormat() const { return _imageFormat; }
        vk::Extent2D extent() const { return _extent; }
        const std::vector<vk::Image>& images() const { return _images; }
        const std::vector<vk::ImageView>& imageViews() const { return _imageViews; }
    };
}