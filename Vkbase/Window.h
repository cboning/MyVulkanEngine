#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <string>
#include <unordered_set>
#include "ResourceBase.h"

namespace Vkbase
{
    class Window : public ResourceBase
    {
    private:
        GLFWwindow *_pWindow;
        std::string _title;
        vk::SurfaceKHR _surface;
        uint32_t _width, _height;
        inline static uint32_t _count = 0;
        inline static std::unordered_set<std::string> _windowNames = {};

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