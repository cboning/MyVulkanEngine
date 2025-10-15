#pragma once
#include "GpuResourceBase.h"

namespace Vkbase
{
class Device;                 // Forward declaration of Device class
struct SurfaceSupportDetails; // Forward declaration of SurfaceSupportDetails struct
class Image;
class Window;

class Swapchain : public GpuResourceBase
{
    friend class ResourceManager;

private:
    vk::SwapchainKHR _swapchain;
    vk::SurfaceFormatKHR _format;
    vk::Extent2D _extent;
    vk::PresentModeKHR _presentMode;
    std::vector<vk::Image> _images;
    std::vector<vk::ImageView> _imageViews;
    std::vector<std::string> _imageNames;
    Window &_window;
    const vk::SurfaceKHR &_surface;
    bool _cleaned = false;

    void init();
    void cleanupTemporary(vk::SwapchainKHR tempSwapchain, const std::vector<vk::ImageView> &tempImageViews);
    void cleanup();
    void determineFormat(SurfaceSupportDetails &details);
    void determineExtent(SurfaceSupportDetails &details);
    void determinePresentMode(SurfaceSupportDetails &details);
    Swapchain(const std::string &resourceName, const std::string &deviceName, const std::string &windowName);
    ~Swapchain() override;

public:
    vk::Format format() const;
    vk::Extent2D extent() const;
    Swapchain *recreate();
    const vk::SwapchainKHR &swapchain() const;
    const std::vector<vk::Image> &images() const;
    const std::vector<std::string> &imageNames() const;
    const std::vector<vk::ImageView> &imageViews() const;
    Device &device();
};
} // namespace Vkbase