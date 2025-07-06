#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_set>
#include "ResourceBase.h"

namespace Vkbase
{
    class Device;
    class Swapchain;
    class CommandPool;
    class Window : public ResourceBase
    {
    private:
        GLFWwindow *_pWindow;
        std::string _title;
        vk::SurfaceKHR _surface;
        uint32_t _width, _height;
        const Device *_pDevice = nullptr;
        const Swapchain *_pSwapchain = nullptr;
        const CommandPool *_pGraphicsCommandPool;
        inline static std::unordered_set<Window *> _delayDestroyWindows;

        void init();
        static void windowClosedCallback(GLFWwindow *pWindow);
    public:
        Window(const std::string &resourceName, std::string title, uint32_t width, uint32_t height);
        ~Window() override;
        const vk::SurfaceKHR &surface() const;
        static void delayDestroy();
        uint32_t width() const;
        uint32_t height() const;
    };
}