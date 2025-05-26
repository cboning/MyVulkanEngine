#include "Window.h"
#include <iostream>

namespace Vkbase
{
    Window::Window(const std::string &resourceName, std::string title, uint32_t width, uint32_t height)
        : ResourceBase(Vkbase::ResourceType::Window, resourceName), _width(800), _height(600), _title("Vulkan Window")
    {
        _windowNames.insert(resourceName);
        init();
        _count++;
    }

    Window::~Window()
    {
        if (_pWindow)
        {
            glfwDestroyWindow(_pWindow);
        }
        if (_surface)
        {
            resourceManager().instance().destroySurfaceKHR(_surface);
        }
        _windowNames.erase(_name);
        _count--;
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
        Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
        window.close();
    }

    void Window::close()
    {
        resourceManager().remove(_resourceType, _name);
    }

    uint32_t Window::count()
    {
        return _count;
    }

    vk::SurfaceKHR &Window::surface()
    {
        return _surface;
    }
}