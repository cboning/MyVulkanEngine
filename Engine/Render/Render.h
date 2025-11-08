#pragma once
#include "../Camera/Camera.h"
#include "../VkGUI/Text.h"
#include "../Vkbase/Vkbase.h"
#include "RenderFrame.h"
#include "RenderObjectManager.h"
#include "RenderPassManager.h"

class Push;
class RenderPassManager;

class Render : public Vkbase::VkResourcesDelegator
{
private:
    Camera _camera = Camera({800, 600});
    Camera _cameraLight = Camera({800, 600});
    std::string _deviceName;

    inline static Vkbase::VkResourceManager &_resourceManager = Vkbase::VkResourceBase::resourceManager();

    Vkbase::VkResourceManagerHolder::WeakReference _renderDelegator = {};

    inline static float _lastTime = glfwGetTime();
    inline static float _currentTime = glfwGetTime();
    inline static float _deltaTime = 0.1f;

    Vkbase::VkResourceManagerHolder::WeakReference _window;

    std::vector<RenderPassManager> _renderPassManagers;
    std::vector<std::shared_ptr<Vkbase::RenderObjectDelegator>> _renderObjects;
    RenderObjectManager _renderObjectManager;
    std::weak_ptr<VkGUI::Text> _fps;

    inline static float _speed = 10.f;
    Push *_pPush = nullptr;
    uint32_t _rootFrameIndex = 0;

    void createRenderPass();
    void createDescriptorSets();
    void createRenderDelegator();
    void recordCommand(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, uint32_t imageIndex, uint32_t currentFrame);
    void cleanup();
    void initWindowEvents();
    static void calcDeltaTime();

public:
    Render(const std::string &windowName);
    static bool shouldEndApplication();
    const std::string &deviceName();
    static void draw();
    void update(uint32_t currentFrame);
};
