#include "Application.h"
#include "Vkbase/ResourceBase.h"
#include "Vkbase/Window.h"

Application::Application()
    : _resourceManager(Vkbase::ResourceBase::resourceManager())
{
    new Vkbase::Window("mainWindow", "Vulkan Window", 800, 600);
}

void Application::run()
{
    mainLoop();
}


void Application::mainLoop()
{
    while (_resourceManager.resources().count(Vkbase::ResourceType::Window))
    {
        glfwPollEvents();
    }
}
void Application::cleanup()
{
    _resourceManager.remove(Vkbase::ResourceType::Window, "mainWindow");
}