#pragma once
#include "Camera/Camera.h"
#include "VkGUI/Font.h"
#include "VkGUI/Text.h"
#include "Vkbase/Vkbase.h"

class Push;

class Render : public Vkbase::ResourcesDelegator
{
private:
    inline static Camera _camera = Camera({800, 600});
    inline static Camera _cameraLight = Camera({800, 600});
    inline static Vkbase::ResourceManager &_resourceManager = Vkbase::ResourceBase::resourceManager();
    Vkbase::RenderDelegator *_pRenderDelegator = nullptr;
    inline static float _lastTime = glfwGetTime();
    inline static float _currentTime = glfwGetTime();
    inline static float _deltaTime = 0.1f;
    Vkbase::Buffer *_pFrameVerticesBuffer = nullptr;
    inline static std::unique_ptr<Font> _pFont = {};
    inline static std::unique_ptr<Text> _pText = {};
    inline static float _speed = 10.f;
    Push *_pPush = nullptr;

    void createRenderPass();
    void createDescriptorSets();
    void createRenderDelegator();
    void resourceInit();
    void recordCommand(Vkbase::CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame);
    void cleanup();
    void renderFrame(Vkbase::CommandBuffer *pCommandBuffer, const std::string &pipelineName, const std::vector<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> &descriptorSet);
    void initWindowEvents();

    static void clacDeltaTime();

public:
    void init();
    static void draw();
    static bool shouldEndApplication();
    Camera &camera();
};
