#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Vkbase
{
class Window;
}

namespace Event
{
typedef std::function<void()> eventFunc;

class KeyInputEvent
{
    friend Vkbase::Window;

  public:
    void addPressedKeyEvent(int key, eventFunc event);
    void addDownKeyEvent(int key, eventFunc event);
    void addUpKeyEvent(int key, eventFunc event);

    void removePressedKeyEvent(int key);
    void removeDownKeyEvent(int key);
    void removeUpKeyEvent(int key);

    static void processing();

  private:
    KeyInputEvent(GLFWwindow *pWindow);
    ~KeyInputEvent();

    void processingEvent();
    void addEvent(int key);
    void removeEvent(int key);

    GLFWwindow *_pWindow;

    std::unordered_map<int, eventFunc> _keyPressedEventMap;
    std::unordered_map<int, eventFunc> _keyDownEventMap;
    std::unordered_map<int, eventFunc> _keyUpEventMap;
    std::unordered_map<int, bool> _keyPressed;

    std::mutex _controlMutex;

    inline static std::mutex _removeMutex = std::mutex();

    inline static std::vector<KeyInputEvent *> _pKeyInputEvents = {};
};
} // namespace Event