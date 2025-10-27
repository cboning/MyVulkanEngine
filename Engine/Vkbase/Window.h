#pragma once
#define GLFW_INCLUDE_VULKAN
#include "../Event/KeyInputEvent.h"
#include "VkGpuResourceBase.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_set>

namespace Event
{
class KeyInputEvent;
}

namespace Vkbase
{
class Device;
class Swapchain;
class Window : public VkGpuResourceBase
{
    friend class VkResourceManager;

public:
    struct InitData
    {
        GLFWwindow *pWindow;
        vk::SurfaceKHR surface;
    };

private:
    struct Deleter
    {
        void operator()(Event::KeyInputEvent *p) const noexcept { delete p; }
    };
    GLFWwindow *_pWindow = nullptr;
    vk::SurfaceKHR _surface;
    uint32_t _width, _height;
    std::string _title;
    const Swapchain *_pSwapchain = nullptr;
    double _cursorPosX, _cursorPosY;
    int _cursorState = GLFW_CURSOR_NORMAL;
    std::unique_ptr<Event::KeyInputEvent, Deleter> _pKeyInputEvent;
    std::function<void(double, double)> _mouseMoveCallback;
    std::function<void(double, double)> _mouseScrollCallback;
    inline static std::unordered_set<Window *> _delayDestroyWindows;
    inline static thread_local std::optional<InitData> _pendingInitData = std::nullopt;

    vk::SurfaceKHR init(uint32_t width, uint32_t height, const std::string &title);
    static void windowClosedCallback(GLFWwindow *pWindow);
    static void mouseMoveCallback(GLFWwindow *pWindow, double xPos, double yPos);
    static void mouseScrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset);
    static void windowResizeCallback(GLFWwindow *pWindow, int width, int height);
    Window(const std::string &resourceName, const std::string &title, uint32_t width, uint32_t height);
    Window(const std::string &resourceName, Device &device, const std::string &title, uint32_t width, uint32_t height);
    static InitData createWindow(const std::string &title, uint32_t width, uint32_t height);

    ~Window() override;

public:
    const vk::SurfaceKHR &surface() const;
    static void delayDestroy();
    uint32_t width() const;
    uint32_t height() const;
    GLFWwindow *window() const;
    void setMouseMoveCallback(const std::function<void(double, double)> &func);
    void setMouseScrollCallback(const std::function<void(double, double)> &func);
    void cursorCapture(int value);
    void switchCursorState();

    Event::KeyInputEvent &keyInputEvent();
};
} // namespace Vkbase