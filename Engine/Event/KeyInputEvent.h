#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Vkbase
{
class Window;
}

namespace Event
{
typedef std::function<void()> EventFunc;

class KeyInputEvent
{
    friend Vkbase::Window;
public:
    void addPressedKeyEvent(int key, EventFunc event);
    void addDownKeyEvent(int key, EventFunc event);
    void addUpKeyEvent(int key, EventFunc event);

    void removePressedKeyEvent(int key);
    void removeDownKeyEvent(int key);
    void removeUpKeyEvent(int key);

    void enable();
    void disable();

    static void processing();

private:
    KeyInputEvent(GLFWwindow *pWindow);
    ~KeyInputEvent();

    void processingEvent();
    void addEvent(int key);
    void removeEvent(int key);

    GLFWwindow *_pWindow = nullptr;

    std::unordered_map<int, EventFunc> _keyPressedEventMap;
    std::unordered_map<int, EventFunc> _keyDownEventMap;
    std::unordered_map<int, EventFunc> _keyUpEventMap;
    std::unordered_map<int, bool> _keyPressed;

    std::mutex _controlMutex;

    inline static std::mutex _removeMutex = std::mutex();

    inline static std::unordered_set<KeyInputEvent *> _pKeyInputEvents = {};
    inline static std::unordered_set<KeyInputEvent *> _pEnabledEvents = {};
};
} // namespace Event