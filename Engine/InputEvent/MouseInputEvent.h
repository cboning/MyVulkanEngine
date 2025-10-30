#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Vkbase
{
class Window;
}

namespace InputEvent
{

typedef std::function<void()> EventFunc;
typedef std::function<void(glm::vec2)> MoveEventFunc;
typedef std::function<void(glm::vec2)> ScrollEventFunc;

enum class Button
{
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE,
    Button4 = GLFW_MOUSE_BUTTON_4,
    Button5 = GLFW_MOUSE_BUTTON_5,
    Button6 = GLFW_MOUSE_BUTTON_6,
    Button7 = GLFW_MOUSE_BUTTON_7,
    Button8 = GLFW_MOUSE_BUTTON_8,
};

inline int toGLFW(Button button) { return static_cast<int>(button); }

class MouseInputEvent
{
    friend Vkbase::Window;

public:
    void addPressedButtonEvent(Button button, EventFunc event);
    void addDownButtonEvent(Button button, EventFunc event);
    void addUpButtonEvent(Button button, EventFunc event);

    void addMoveEvent(MoveEventFunc event);
    void addScrollEvent(ScrollEventFunc event);

    void removePressedButtonEvent(Button button);
    void removeDownButtonEvent(Button button);
    void removeUpButtonEvent(Button button);

    void enable();
    void disable();

    glm::vec2 pos();

    static void processing();

private:
    MouseInputEvent(GLFWwindow *pWindow);
    ~MouseInputEvent();

    void processingEvent();
    void addEvent(Button button);
    void removeEvent(Button button);

    GLFWwindow *_pWindow = nullptr;

    std::unordered_map<Button, std::vector<EventFunc>> _buttonPressedEventMap;
    std::unordered_map<Button, std::vector<EventFunc>> _buttonDownEventMap;
    std::unordered_map<Button, std::vector<EventFunc>> _buttonUpEventMap;
    std::unordered_map<Button, bool> _buttonPressed;

    std::vector<MoveEventFunc> _moveEvents;
    std::vector<ScrollEventFunc> _scrollEvents;

    glm::vec2 _cursorPos{0.0f};
    glm::vec2 _scrollOffset{0.0f};

    std::mutex _controlMutex;

    inline static std::mutex _removeMutex = std::mutex();
    inline static std::unordered_set<MouseInputEvent *> _pMouseInputEvents = {};
    inline static std::unordered_set<MouseInputEvent *> _pEnabledEvents = {};
};

} // namespace InputEvent
