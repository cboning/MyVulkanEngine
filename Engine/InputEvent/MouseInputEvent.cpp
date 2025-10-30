#include "MouseInputEvent.h"
#include <algorithm>
#include <iostream>

namespace InputEvent
{
static std::unordered_map<GLFWwindow *, MouseInputEvent *> g_mouseInstances;

MouseInputEvent::MouseInputEvent(GLFWwindow *pWindow) : _pWindow(pWindow)
{
    _pMouseInputEvents.insert(this);
    g_mouseInstances[pWindow] = this;
    enable();

    glfwSetCursorPosCallback(pWindow,
                             [](GLFWwindow *window, double x, double y)
                             {
                                 auto it = g_mouseInstances.find(window);
                                 if (it == g_mouseInstances.end())
                                     return;
                                 auto *self = it->second;
                                 self->_cursorPos = glm::vec2(x, y);
                                 for (auto &cb : self->_moveEvents)
                                     cb(self->_cursorPos);
                             });

    glfwSetScrollCallback(pWindow,
                          [](GLFWwindow *window, double xoffset, double yoffset)
                          {
                              auto it = g_mouseInstances.find(window);
                              if (it == g_mouseInstances.end())
                                  return;
                              auto *self = it->second;
                              self->_scrollOffset = glm::vec2(xoffset, yoffset);
                              for (auto &cb : self->_scrollEvents)
                                  cb(self->_scrollOffset);
                          });
}

MouseInputEvent::~MouseInputEvent()
{
    std::unique_lock<std::mutex> lock(_removeMutex);
    _pMouseInputEvents.erase(this);
    g_mouseInstances.erase(_pWindow);
}

void MouseInputEvent::processing()
{
    std::unique_lock<std::mutex> lock(_removeMutex);
    for (MouseInputEvent *pMouse : _pMouseInputEvents)
        pMouse->processingEvent();
}

void MouseInputEvent::processingEvent()
{
    for (auto &[button, pressed] : _buttonPressed)
    {
        int glfwButton = toGLFW(button);

        if (glfwGetMouseButton(_pWindow, glfwButton) == GLFW_PRESS)
        {
            if (!pressed)
            {
                auto it = _buttonDownEventMap.find(button);
                if (it != _buttonDownEventMap.end())
                    for (auto &t : it->second)
                        t();
            }

            auto it = _buttonPressedEventMap.find(button);
            if (it != _buttonPressedEventMap.end())
                for (auto &t : it->second)
                    t();

            pressed = true;
        }
        else
        {
            if (pressed)
            {
                auto it = _buttonUpEventMap.find(button);
                if (it != _buttonUpEventMap.end())
                    for (auto &t : it->second)
                        t();
            }
            pressed = false;
        }
    }
}

void MouseInputEvent::addPressedButtonEvent(Button button, EventFunc event)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    addEvent(button);
    _buttonPressedEventMap[button].push_back(event);
}

void MouseInputEvent::addDownButtonEvent(Button button, EventFunc event)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    addEvent(button);
    _buttonDownEventMap[button].push_back(event);
}

void MouseInputEvent::addUpButtonEvent(Button button, EventFunc event)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    addEvent(button);
    _buttonUpEventMap[button].push_back(event);
}

void MouseInputEvent::addMoveEvent(MoveEventFunc event)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _moveEvents.push_back(event);
}

void MouseInputEvent::addScrollEvent(ScrollEventFunc event)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _scrollEvents.push_back(event);
}

void MouseInputEvent::removePressedButtonEvent(Button button)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _buttonPressedEventMap.erase(button);
    removeEvent(button);
}

void MouseInputEvent::removeDownButtonEvent(Button button)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _buttonDownEventMap.erase(button);
    removeEvent(button);
}

void MouseInputEvent::removeUpButtonEvent(Button button)
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _buttonUpEventMap.erase(button);
    removeEvent(button);
}

void MouseInputEvent::enable()
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _pEnabledEvents.insert(this);
}

void MouseInputEvent::disable()
{
    std::unique_lock<std::mutex> lock(_controlMutex);
    _pEnabledEvents.erase(this);
}

glm::vec2 MouseInputEvent::pos() { return _cursorPos; }

void MouseInputEvent::addEvent(Button button)
{
    if (_buttonPressed.find(button) == _buttonPressed.end())
    {
        _buttonPressed[button] = (glfwGetMouseButton(_pWindow, toGLFW(button)) == GLFW_PRESS);
    }
}

void MouseInputEvent::removeEvent(Button button)
{
    if (_buttonPressedEventMap.count(button))
        return;
    if (_buttonDownEventMap.count(button))
        return;
    if (_buttonUpEventMap.count(button))
        return;
    _buttonPressed.erase(button);
}

} // namespace InputEvent
