#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <string>
#include <unordered_set>
#include "ResourceBase.h"

namespace Vkbase
{
    class Device;
    class Swapchain;
    class Window : public ResourceBase
    {
    private:
        GLFWwindow *_pWindow;
        std::string _title;
        vk::SurfaceKHR _surface;
        uint32_t _width, _height;
        Device *_pDevice = nullptr;
        Swapchain *_pSwapchain = nullptr;

        inline static uint32_t _count = 0;

        void init();
        void close();
        static void windowClosedCallback(GLFWwindow *pWindow);
    public:
        Window(const std::string &resourceName, std::string title, uint32_t width, uint32_t height);
        ~Window() override;
        
        static uint32_t count();
        vk::SurfaceKHR &surface();
    };
}