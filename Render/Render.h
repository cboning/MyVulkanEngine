#pragma once
#include "../Camera/Camera.h"
#include "../VkGUI/Font.h"
#include "../VkGUI/Text.h"
#include "../Vkbase/Vkbase.h"
#include "RenderFrame.h"
#include "RenderPassManager.h"
#include "RenderObjectManager.h"

class Push;
class RenderPassManager;

class Render : public Vkbase::VkResourcesDelegator
{
private:
    Camera _camera = Camera({800, 600});
    Camera _cameraLight = Camera({800, 600});
    std::string _deviceName;

    inline static Vkbase::VkResourceManager &_resourceManager = Vkbase::VkResourceBase::resourceManager();

    Vkbase::RenderDelegator *_pRenderDelegator = nullptr;

    

    inline static float _lastTime = glfwGetTime();
    inline static float _currentTime = glfwGetTime();
    inline static float _deltaTime = 0.1f;
    inline static std::weak_ptr<Text> _pText = {};

    std::vector<RenderPassManager> _renderPassManagers;
    std::vector<std::shared_ptr<Vkbase::RenderObjectDelegator>> _renderObjects;
    RenderObjectManager _renderObjectManager;

    inline static float _speed = 10.f;
    Push *_pPush = nullptr;

    Vkbase::Window *_pWindow;

    inline static bool _inited = false;

    void createRenderPass();
    void createDescriptorSets();
    void createRenderDelegator();
    void recordCommand(Vkbase::CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame);
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
