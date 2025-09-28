#include "KeyInputEvent.h"
#include <algorithm>

namespace Event
{

KeyInputEvent::KeyInputEvent(GLFWwindow *pWindow) : _pWindow(pWindow)
{
    _pKeyInputEvents.insert(this);
    enable();
}

KeyInputEvent::~KeyInputEvent()
{
    _removeMutex.lock();
    _pKeyInputEvents.erase(this);
    _removeMutex.unlock();
}

void KeyInputEvent::processing()
{
    _removeMutex.lock();
    for (KeyInputEvent *pKeyInputevent : _pKeyInputEvents)
        pKeyInputevent->processingEvent();
    _removeMutex.unlock();
}

void KeyInputEvent::processingEvent()
{
    for (std::pair<const int, bool> &keyState : _keyPressed)
    {
        if (glfwGetKey(_pWindow, keyState.first) == GLFW_PRESS)
        {
            if (!keyState.second)
            {
                auto it = _keyDownEventMap.find(keyState.first);
                if (it != _keyDownEventMap.end())
                    it->second();
            }

            auto it = _keyPressedEventMap.find(keyState.first);
            if (it != _keyPressedEventMap.end())
                it->second();

            keyState.second = true;
        }
        else
        {
            if (keyState.second)
            {
                auto it = _keyUpEventMap.find(keyState.first);
                if (it != _keyUpEventMap.end())
                    it->second();
            }
            keyState.second = false;
        }
    }
}

void KeyInputEvent::addPressedKeyEvent(int key, EventFunc event)
{
    std::lock_guard<std::mutex> lock(_controlMutex);
    addEvent(key);
    _keyPressedEventMap[key] = event;
}
void KeyInputEvent::addDownKeyEvent(int key, EventFunc event)
{
    std::lock_guard<std::mutex> lock(_controlMutex);
    addEvent(key);
    _keyDownEventMap[key] = event;
}
void KeyInputEvent::addUpKeyEvent(int key, EventFunc event)
{
    std::lock_guard<std::mutex> lock(_controlMutex);
    addEvent(key);
    _keyUpEventMap[key] = event;
}

void KeyInputEvent::removePressedKeyEvent(int key)
{
    std::lock_guard<std::mutex> lock(_controlMutex);
    _keyPressedEventMap.erase(key);
    removeEvent(key);
}
void KeyInputEvent::removeDownKeyEvent(int key)
{
    std::lock_guard<std::mutex> lock(_controlMutex);
    _keyDownEventMap.erase(key);
    removeEvent(key);
}
void KeyInputEvent::removeUpKeyEvent(int key)
{
    std::lock_guard<std::mutex> lock(_controlMutex);
    _keyUpEventMap.erase(key);
    removeEvent(key);
}

void KeyInputEvent::enable() { _pEnabledEvents.insert(this); }

void KeyInputEvent::disable() { _pEnabledEvents.erase(this); }

void KeyInputEvent::addEvent(int key)
{
    auto it = _keyPressed.find(key);
    if (it == _keyPressed.end())
    {
        if (glfwGetKey(_pWindow, key) == GLFW_PRESS)
        {
            _keyPressed[key] = true;
        }
        else
        {
            _keyPressed[key] = false;
        }
    }
}

void KeyInputEvent::removeEvent(int key)
{
    if (_keyPressedEventMap.count(key))
        return;

    if (_keyDownEventMap.count(key))
        return;

    if (_keyUpEventMap.count(key))
        return;

    _keyPressed.erase(key);
}

} // namespace Event