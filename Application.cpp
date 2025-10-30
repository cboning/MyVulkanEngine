#include "Application.h"
#include "Engine/Modelbase/Modelbase.h"
#include "Data.h"
#include "Engine/EngineLogic.h"
#include "Engine/Data/Octree.h"
#include "Engine/Resources/ResourceManager.h"

Application::Application()
    : _renderer(std::make_unique<Render>("mainWindow"))
{
    init();
}

void Application::init()
{
}

void Application::run()
{
    mainLoop();
}

void Application::mainLoop()
{
    EngineLogic engine;
    engine.run();

    while (Render::shouldEndApplication())
    {
        Render::draw();
    }
    engine.stop();

    _renderer.reset();
    Resources::ResourceManager::shutDown();
    Vkbase::VkResourceManager::shutDown();
}