#include "Window.h"
#include "CommandPool.h"
#include "Device.h"
#include "Swapchain.h"
#include <iostream>

namespace Vkbase
{
Window::Window(const std::string &resourceName, const std::string &title, uint32_t width, uint32_t height)
    : VkGpuResourceBase(Vkbase::VkResourceType::Window, resourceName, *Device::getSuitableDevice(createWindow(title, width, height).surface)), _width(width),
      _height(height), _title(title)
{
    if (!_pendingInitData.has_value())
        throw std::runtime_error("[Error] No pending window data found (preInit missing)");

    _pWindow = _pendingInitData->pWindow;
    _surface = _pendingInitData->surface;
    _pendingInitData.reset();

    _pKeyInputEvent = std::unique_ptr<InputEvent::KeyInputEvent, Deleter>(new InputEvent::KeyInputEvent(_pWindow));
    _pMouseInputEvent = std::unique_ptr<InputEvent::MouseInputEvent, Deleter>(new InputEvent::MouseInputEvent(_pWindow));
    _pMouseInputEvent->addMoveEvent([this](glm::vec2 pos) { this->mouseMoveCallback(pos); });
    _pMouseInputEvent->addScrollEvent([this](glm::vec2 offset) { this->mouseScrollCallback(offset); });

    cursorCapture(_cursorState);
    // Set the user pointer to this window instance
    glfwSetWindowUserPointer(_pWindow, this);
    // Set the close callback to handle window close events
    glfwSetWindowCloseCallback(_pWindow, windowClosedCallback);
    glfwSetWindowSizeCallback(_pWindow, windowResizeCallback);

    if (_surface)
    {
        connectTo(&CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Graphics));
        connectTo(&CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Compute));
        connectTo(&CommandPool::getCommandPool(_device.name(), Vkbase::CommandPoolQueueType::Present));

        _pSwapchain = createResource<Swapchain>(name(), _device.name(), name());
    }
}

Window::InitData Window::createWindow(const std::string &title, uint32_t width, uint32_t height)
{
    GLFWwindow *pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!pWindow)
        throw std::runtime_error("[Error] Failed to create GLFW window");

    VkSurfaceKHR rawSurface{};
    VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(resourceManager().vkInstance()), pWindow, nullptr, &rawSurface);

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

void Window::mouseMoveCallback(glm::vec2 pos)
{
    if (this->_mouseMoveCallback)
        this->_mouseMoveCallback(pos.x - this->_cursorPosX, pos.y - this->_cursorPosY);
    this->_cursorPosX = pos.x;
    this->_cursorPosY = pos.y;
}

void Window::mouseScrollCallback(glm::vec2 offset)
{
    if (this->_mouseScrollCallback)
        this->_mouseScrollCallback(offset.x, offset.y);
}

void Window::windowResizeCallback(GLFWwindow *pWindow, int width, int height)
{
    Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
    window._width = width;
    window._height = height;
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

InputEvent::KeyInputEvent &Window::keyInputEvent() { return *_pKeyInputEvent; }
InputEvent::MouseInputEvent &Window::mouseInputEvent() { return *_pMouseInputEvent; }
} // namespace Vkbase