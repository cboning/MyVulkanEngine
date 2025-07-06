#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Vkbase/ResourceManager.h"
#include "Vkbase/RenderDelegator.h"

class Application
{
private:
    Vkbase::ResourceManager &_resourceManager;
    Vkbase::RenderDelegator *_pRenderDelegator;
    void init();
    void createRenderPass();
    void createRenderDelegator();
    void cleanup();
    void mainLoop();

public:
    Application();
    void run();
    static void recordCommand(Vkbase::ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex);
};
