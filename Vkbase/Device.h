#pragma once
#include <unordered_set>
#include "ResourceBase.h"


namespace Vkbase
{
    struct SurfaceSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };
    class CommandPool;
    enum class CommandPoolQueueType;

    class Device : public ResourceBase
    {
    public:
        struct QueueFamilyIndices
        {
            int graphicsFamilyIndex = -1;
            int computeFamilyIndex = -1;
            int presentFamilyIndex = -1;

            bool isComplete() const
            {
                return graphicsFamilyIndex >= 0 && presentFamilyIndex >= 0 && computeFamilyIndex >= 0;
            }

            bool operator==(const QueueFamilyIndices &right) const
            {
                return graphicsFamilyIndex == right.graphicsFamilyIndex && computeFamilyIndex == right.computeFamilyIndex && presentFamilyIndex == right.presentFamilyIndex;
            }
        };

        Device(const std::string &resourceName, const vk::SurfaceKHR &surface);
        ~Device() override;

        static Device *getSuitableDevice(const vk::SurfaceKHR &surface);
        static SurfaceSupportDetails querySwapChainSupport(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

        const vk::PhysicalDevice &physicalDevice() const;
        const vk::Device &device() const;
        const vk::Queue &graphicsQueue() const;
        const vk::Queue &presentQueue() const;
        const vk::Queue &computeQueue() const;
        const QueueFamilyIndices &queueFamilyIndices() const;
        vk::Format findSupportedFormat(std::vector<vk::Format> formats, vk::ImageTiling tiling, vk::FormatFeatureFlags feature) const;
    
    private:
        vk::Device _device;
        vk::PhysicalDevice _physicalDevice;
        vk::Queue _graphicsQueue;
        vk::Queue _presentQueue;
        vk::Queue _computeQueue;
        inline static std::vector<const char *> _extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
        inline static std::vector<const char *> _layers;
        QueueFamilyIndices _queueFamilyIndices;
        std::unordered_set<Device *> _pDevice;

        void createLogicalDevice();
        static vk::PhysicalDevice pickPhysicalDevice(const vk::SurfaceKHR &surface);
        static bool isPhysicalDeviceSuitable(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
        static bool checkSwapChainSupport(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
        static bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);
        static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
    };
}