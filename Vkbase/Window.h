#pragma once
#define GLFW_INCLUDE_VULKAN
#include "ResourceBase.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_set>

namespace Vkbase
{
class Device;
class Swapchain;
class Window : public ResourceBase
{
    friend class ResourceManager;

  private:
    GLFWwindow *_pWindow;
    vk::SurfaceKHR _surface;
    uint32_t _width, _height;
    std::string _title;
    const Device *_pDevice = nullptr;
    const Swapchain *_pSwapchain = nullptr;
    double _cursorPosX, _cursorPosY;
    int _cursorState = GLFW_CURSOR_NORMAL;
    std::function<void(double, double)> _mouseMoveCallback;
    inline static std::unordered_set<Window *> _delayDestroyWindows;

    void init();
    static void windowClosedCallback(GLFWwindow *pWindow);
    static void mouseMoveCallback(GLFWwindow *pWindow, double xPos, double yPos);
    Window(const std::string &resourceName, const std::string &title, uint32_t width, uint32_t height);
    ~Window() override;

  public:
    const vk::SurfaceKHR &surface() const;
    static void delayDestroy();
    uint32_t width() const;
    uint32_t height() const;
    GLFWwindow *window() const;
    void setMouseMoveCallback(const std::function<void(double, double)> &func);
    void cursorCapture(int value);
    void switchCursorState();
};
} // namespace Vkbase