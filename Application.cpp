#include "Application.h"
#include "Vkbase/ResourceBase.h"
#include "Vkbase/Window.h"



Application::Application()
{
    init();
    new Vkbase::Window("mainWindow", "Vulkan Window", 800, 600);
}

void Application::init()
{
    if (!glfwInit())
    {
        std::cerr << "[Error]Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void Application::run()
{
    mainLoop();
}


void Application::mainLoop()
{
    while (Vkbase::Window::count())
    {
        glfwPollEvents();
    }
}
void Application::cleanup()
{
    Vkbase::ResourceBase::resourceManager().remove(Vkbase::ResourceType::Window, "mainWindow");
}