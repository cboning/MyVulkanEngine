#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Vkbase/ResourceManager.h"

class Application
{
private:
    Vkbase::ResourceManager &_resourceManager;
    void cleanup();
    void mainLoop();

public:
    Application();
    void run();
};
