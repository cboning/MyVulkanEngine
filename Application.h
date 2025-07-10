#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Vkbase/ResourceManager.h"
#include "Vkbase/RenderDelegator.h"
#include "Camera.h"

class Application
{
private:
    Vkbase::ResourceManager &_resourceManager;
    Vkbase::RenderDelegator *_pRenderDelegator;
    static Camera _camera;
    float _lastTime = glfwGetTime();
    float _currentTime = glfwGetTime();
    float _deltaTime = 0.1f;
    void init();
    void createRenderPass();
    void createRenderDelegator();
    static void updateUniformBuffer(Vkbase::ResourceManager &resourceManager, uint32_t index);
    void cleanup();
    void mainLoop();
    void createDescriptorSets();
    void processInputs();

public:
    Application();
    void run();
    void recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame);
};