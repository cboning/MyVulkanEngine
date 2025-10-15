#include "Window.h"
#include "../Event/KeyInputEvent.h"
#include "CommandPool.h"
#include "Device.h"
#include "Swapchain.h"
#include <iostream>

namespace Vkbase
{
Window::Window(const std::string &resourceName, const std::string &title, uint32_t width, uint32_t height)
    : GpuResourceBase(Vkbase::ResourceType::Window, resourceName, *Device::getSuitableDevice(createWindow(title, width, height).surface)), _width(width),
      _height(height), _title(title)
{
    if (!_pendingInitData.has_value())
        throw std::runtime_error("[Error] No pending window data found (preInit missing)");

    _pWindow = _pendingInitData->pWindow;
    _surface = _pendingInitData->surface;
    _pendingInitData.reset();

    _pKeyInputEvent = std::unique_ptr<Event::KeyInputEvent, Deleter>(new Event::KeyInputEvent(_pWindow));

    cursorCapture(_cursorState);
    // Set the user pointer to this window instance
    glfwSetWindowUserPointer(_pWindow, this);
    // Set the close callback to handle window close events
    glfwSetWindowCloseCallback(_pWindow, windowClosedCallback);
    glfwSetCursorPosCallback(_pWindow, mouseMoveCallback);
    glfwSetScrollCallback(_pWindow, mouseScrollCallback);

    if (_surface)
    {
        connectTo(&CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Graphics));
        connectTo(&CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Compute));
        connectTo(&CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Present));

        _pSwapchain = createResource<Swapchain>(resourceName, _device.name(), resourceName);
    }
}

Window::InitData Window::createWindow(const std::string &title, uint32_t width, uint32_t height)
{
    GLFWwindow *pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!pWindow)
        throw std::runtime_error("[Error] Failed to create GLFW window");

    VkSurfaceKHR rawSurface{};
    VkResult result = glfwCreateWindowSurface(resourceManager().vkInstance(), pWindow, nullptr, &rawSurface);

    if (result != VK_SUCCESS)
        throw std::runtime_error("[Error] Failed to create Vulkan surface");

    vk::SurfaceKHR surface = rawSurface;

    _pendingInitData = InitData{pWindow, surface};

    return _pendingInitData.value();
}

Window::~Window()
{
    if (_pWindow)
    {
        glfwSetCursorPosCallback(_pWindow, nullptr);
        glfwSetScrollCallback(_pWindow, nullptr);
        glfwSetWindowCloseCallback(_pWindow, nullptr);
        glfwSetWindowUserPointer(_pWindow, nullptr);
    }
    _onDelayDestroy = [pWindow = _pWindow, surface = _surface]()
    {
        if (pWindow)
            glfwDestroyWindow(pWindow);

        if (surface)
            resourceManager().vkInstance().destroySurfaceKHR(surface);
    };
}

vk::SurfaceKHR Window::init(uint32_t width, uint32_t height, const std::string &title)
{
    _pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!_pWindow)
        throw std::runtime_error("[Error] Failed to create GLFW window");
    // Create Vulkan surface
    glfwCreateWindowSurface(resourceManager().vkInstance(), _pWindow, nullptr, reinterpret_cast<VkSurfaceKHR *>(&_surface));

    if (!_surface)
        throw std::runtime_error("[Error] Failed to create Vulkan surface");

    return _surface;
}

void Window::windowClosedCallback(GLFWwindow *pWindow) { _delayDestroyWindows.insert(static_cast<Window *>(glfwGetWindowUserPointer(pWindow))); }

void Window::mouseMoveCallback(GLFWwindow *pWindow, double xPos, double yPos)
{
    Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
    if (window._mouseMoveCallback)
        window._mouseMoveCallback(xPos - window._cursorPosX, yPos - window._cursorPosY);
    window._cursorPosX = xPos;
    window._cursorPosY = yPos;
}

void Window::mouseScrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset)
{
    Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
    if (window._mouseScrollCallback)
        window._mouseScrollCallback(xOffset, yOffset);
}

void Window::setMouseMoveCallback(const std::function<void(double, double)> &func) { _mouseMoveCallback = func; }

void Window::setMouseScrollCallback(const std::function<void(double, double)> &func) { _mouseScrollCallback = func; }

const vk::SurfaceKHR &Window::surface() const { return _surface; }

void Window::delayDestroy()
{
    for (Window *pWindow : _delayDestroyWindows)
        pWindow->destroy();
    _delayDestroyWindows.clear();
}

uint32_t Window::width() const { return _width; }

uint32_t Window::height() const { return _height; }

GLFWwindow *Window::window() const { return _pWindow; }

void Window::cursorCapture(int value) { glfwSetInputMode(_pWindow, GLFW_CURSOR, value); }

void Window::switchCursorState()
{
    _cursorState = _cursorState == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
    cursorCapture(_cursorState);
}

Event::KeyInputEvent &Window::keyInputEvent() { return *_pKeyInputEvent; }
} // namespace Vkbase