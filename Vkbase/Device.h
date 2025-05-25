#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include <unordered_set>
#include "ResourceBase.h"


namespace Vkbase
{
    class Device : public ResourceBase
    {
    public:
        struct QueueFamilyIndices
        {
            int graphicsFamilyIndex = -1;
            int computeFamilyIndex = -1;
            int presentFamilyIndex = -1;

            bool isComplete()
            {
                return graphicsFamilyIndex >= 0 && presentFamilyIndex >= 0 && computeFamilyIndex >= 0;
            }
        };
        struct SurfaceSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        };
        
        Device(const std::string &resourceName, vk::SurfaceKHR surface);
        ~Device() override;

        SurfaceSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);
        vk::Device device();
        vk::PhysicalDevice physicalDevice();
        vk::Queue graphicsQueue();
        vk::Queue presentQueue();
        QueueFamilyIndices queueFamilyIndices();
    
    private:

        vk::Device _device;
        vk::PhysicalDevice _physicalDevice;
        vk::Queue _graphicsQueue;
        vk::Queue _presentQueue;
        std::vector<const char *> _extensions;
        std::vector<const char *> _layers;
        QueueFamilyIndices _queueFamilyIndices;

        void createLogicalDevice();
        void pickPhysicalDevice(vk::SurfaceKHR surface);
        bool isphysicalDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);
        bool checkSwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);
        bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

        QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
        // static 
        
    };
}