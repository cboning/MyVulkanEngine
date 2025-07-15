#pragma once
#include "Vkbase/Vkbase.h"
#include "Camera.h"

class Render
{
private:

    inline static Camera _camera = Camera({800, 600});
    inline static Vkbase::ResourceManager &_resourceManager = Vkbase::ResourceBase::resourceManager();
    Vkbase::RenderDelegator *_pRenderDelegator;
    inline static float _lastTime = glfwGetTime();
    inline static float _currentTime = glfwGetTime();
    inline static float _deltaTime = 0.1f;

    void createRenderPass();
    void createDescriptorSets();
    void createRenderDelegator();
    void resourceInit();
    void updateUniformBuffer(Vkbase::ResourceManager &resourceManager, uint32_t index);
    void recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame);
    void cleanup();
    static void processInputs();

public:
    void init();
    static void draw();
    static bool shouldEndApplication();
    Camera &camera();
};


