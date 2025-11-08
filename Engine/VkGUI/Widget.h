#pragma once

#include "../Vkbase/Mesh.h"
#include "../Vkbase/RenderObjectDelegator.h"
#include <glm/glm.hpp>

namespace VkGUI
{
struct WidgetUBOData
{
    glm::mat4 projection;
    glm::mat4 model;
};

struct WidgetUBOFragData
{
    glm::vec4 color;
    glm::u32vec4 rect;
    glm::u32vec4 type;
};

enum class WidgetType
{
    Character,
    Frame,
    Image
};

enum class MouseEventType
{
    Move,
    Down,
    Up
};

enum class MouseState
{
    Normal,
    Above,
    Clicked
};

class Widget : public Vkbase::RenderObjectDelegator, public std::enable_shared_from_this<Widget>
{
public:
    struct Vertex
    {
        glm::vec4 posTex;

        static vk::VertexInputBindingDescription bindingDescription()
        {
            vk::VertexInputBindingDescription description;
            description.setBinding(0).setStride(sizeof(Vertex)).setInputRate(vk::VertexInputRate::eVertex);

            return description;
        }

        static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
        {
            std::vector<vk::VertexInputAttributeDescription> descriptions;
            descriptions.resize(1);

            descriptions[0].setBinding(0).setFormat(vk::Format::eR32G32B32A32Sfloat).setLocation(0).setOffset(offsetof(Vertex, posTex));

            return descriptions;
        }
    };

protected:
    MouseState mouseState() const;
    virtual void onUpdate();

private:
    struct Deleter
    {
        template <typename T> void operator()(T *pT) { delete pT; }
    };
    std::weak_ptr<Widget> _superWidget = {};

    std::vector<std::shared_ptr<Widget>> _subWidgets = {};

    std::string _vertUBOName = "";
    std::string _fragUBOName = "";

    std::shared_ptr<Vkbase::Mesh<Vertex>> _mesh;
    glm::vec4 _color = glm::vec4(0.0f);
    const WidgetType _type;
    glm::mat4 _model = glm::mat4(1.0f);
    glm::u32vec4 _rect = glm::u32vec4(0);
    glm::u32vec4 _renderRect = glm::u32vec4(0);
    std::function<void()> _commandShouldRecordFunc = {};

    glm::vec4 _clickedColorFactor = glm::vec4(1.0f);
    glm::vec4 _aboveColorFactor = glm::vec4(1.0f);

    MouseState _mouseState = MouseState::Normal;
    bool _trigger = false;

    std::string _textureName;

    template <typename T> std::weak_ptr<T> add(std::shared_ptr<T> &subWidget);

    inline static glm::mat4 _projection = glm::mat4(1.0f);
    inline static std::unordered_map<Vkbase::VkResourceManagerHolder::WeakReference, std::vector<std::weak_ptr<Widget>>> _rootWidgets = {};

    void updateRenderRect();
    void buildMesh();
    void processMouse(glm::vec2 pos, MouseEventType eventType);
    bool inRect(glm::vec2 pos);
    void init();

public:
    Widget(WidgetType type, const std::string &deviceName, const std::string &textureName = "");
    virtual ~Widget();
    template <typename T, typename... Args> std::weak_ptr<T> create(Args &&...args);
    template <typename T, typename... Args>
    static std::shared_ptr<T> create(const Vkbase::VkResourceManagerHolder::WeakReference &window, const std::string &deviceName, Args &&...args);
    void destroy();

    void setColor(const glm::vec4 &color);

    void setRect(const glm::u32vec4 &rect);

    const glm::u32vec4 &rect() const;

    void onDraw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName,
                uint32_t imageIndex, uint32_t frameIndex) const override;

    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets) override;
    void writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets) override;
    const vk::DescriptorSetLayout &descriptorSetsLayout();
    void setCommandShouldRecordFunc(const std::function<void()> &func);

    static void setScreenSize(const glm::u32vec2 &screenSize);
};

template <typename T> inline std::weak_ptr<T> Widget::add(std::shared_ptr<T> &subWidget)
{
    subWidget->_superWidget = shared_from_this();
    _subWidgets.push_back(subWidget);
    subWidget->setCommandShouldRecordFunc(_commandShouldRecordFunc);
    subWidget->init();
    return std::weak_ptr<T>(subWidget);
}

template <typename T, typename... Args> inline std::weak_ptr<T> Widget::create(Args &&...args)
{
    static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");

    auto subWidget = std::shared_ptr<T>(new T(deviceName(), std::forward<Args>(args)...));
    return add(subWidget);
}

template <typename T, typename... Args>
inline std::shared_ptr<T> Widget::create(const Vkbase::VkResourceManagerHolder::WeakReference &window, const std::string &deviceName, Args &&...args)
{
    auto widget = std::shared_ptr<T>(new T(deviceName, std::forward<Args>(args)...));
    if (auto pWindow = window.lock<Vkbase::Window>())
    {
        if (!_rootWidgets.count(window))
        {
            pWindow->mouseInputEvent().addMoveEvent(
                [pWindow, window](glm::vec2)
                {
                    for (auto &rootWidget : _rootWidgets[window])
                        if (auto p = rootWidget.lock())
                        {
                            int width, height;
                            glfwGetFramebufferSize(pWindow->window(), &width, &height);
                            p->processMouse(pWindow->mouseInputEvent().pos() * glm::vec2(width / pWindow->width(), height / pWindow->height()),
                                            MouseEventType::Move);
                        }
                });
            pWindow->mouseInputEvent().addDownButtonEvent(InputEvent::Button::Left,
                                                          [pWindow, window]()
                                                          {
                                                              for (auto &rootWidget : _rootWidgets[window])
                                                                  if (auto p = rootWidget.lock())
                                                                  {
                                                                      int width, height;
                                                                      glfwGetFramebufferSize(pWindow->window(), &width, &height);
                                                                      p->processMouse(pWindow->mouseInputEvent().pos() *
                                                                                          glm::vec2(width / pWindow->width(), height / pWindow->height()),
                                                                                      MouseEventType::Down);
                                                                  }
                                                          });
            pWindow->mouseInputEvent().addUpButtonEvent(InputEvent::Button::Left,
                                                        [pWindow, window]()
                                                        {
                                                            for (auto &rootWidget : _rootWidgets[window])
                                                                if (auto p = rootWidget.lock())
                                                                {
                                                                    int width, height;
                                                                    glfwGetFramebufferSize(pWindow->window(), &width, &height);
                                                                    p->processMouse(pWindow->mouseInputEvent().pos() *
                                                                                        glm::vec2(width / pWindow->width(), height / pWindow->height()),
                                                                                    MouseEventType::Up);
                                                                }
                                                        });
        }
        _rootWidgets[window].push_back(std::dynamic_pointer_cast<Widget>(widget));
    }
    widget->init();
    return widget;
}
} // namespace VkGUI