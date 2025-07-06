#include "Window.h"
#include "Device.h"
#include "Swapchain.h"
#include "CommandPool.h"
#include <iostream>

namespace Vkbase
{
    Window::Window(const std::string &resourceName, std::string title, uint32_t width, uint32_t height)
        : ResourceBase(Vkbase::ResourceType::Window, resourceName), _width(width), _height(height), _title("Vulkan Window")
    {
        init();
        if (_surface)
        {
            _pDevice = connectTo(Device::getSuitableDevice(_surface));
            connectTo(&CommandPool::getCommandPool(_pDevice->name(), Vkbase::CommandPoolQueueType::Graphics));
            connectTo(&CommandPool::getCommandPool(_pDevice->name(), Vkbase::CommandPoolQueueType::Compute));
            connectTo(&CommandPool::getCommandPool(_pDevice->name(), Vkbase::CommandPoolQueueType::Present));

            _pSwapchain = new Swapchain(resourceName, _pDevice->name(), resourceName);
        }
    }

    Window::~Window()
    {
        if (_pWindow)
            glfwDestroyWindow(_pWindow);

        if (_surface)
            resourceManager().instance().destroySurfaceKHR(_surface);
    }

    void Window::init()
    {
        _pWindow = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);
        if (!_pWindow)
        {
            std::cerr << "[Error] Failed to create GLFW window" << std::endl;
            resourceManager().remove(_resourceType, _name);
            return;
        }
        // Set the user pointer to this window instance
        glfwSetWindowUserPointer(_pWindow, this);
        // Set the close callback to handle window close events
        glfwSetWindowCloseCallback(_pWindow, windowClosedCallback);

        // Create Vulkan surface
        glfwCreateWindowSurface(resourceManager().instance(), _pWindow, nullptr, reinterpret_cast<VkSurfaceKHR *>(&_surface));
        if (!_surface)
        {
            std::cerr << "[Error] Failed to create Vulkan surface" << std::endl;
            resourceManager().remove(_resourceType, _name);
            return;
        }
    }

    void Window::windowClosedCallback(GLFWwindow *pWindow)
    {
        _delayDestroyWindows.insert(static_cast<Window *>(glfwGetWindowUserPointer(pWindow)));
    }

    const vk::SurfaceKHR &Window::surface() const
    {
        return _surface;
    }

    void Window::delayDestroy()
    {
        for (Window *pWindow : _delayDestroyWindows)
            pWindow->destroy();
        _delayDestroyWindows.clear();
    }

    uint32_t Window::width() const
    {
        return _width;
    }
    uint32_t Window::height() const
    {
        return _height;
    }
}