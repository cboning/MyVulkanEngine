#include "Application.h"
#include "Modelbase/Modelbase.h"
#include "Data.h"
#include "Engine/EngineLogic.h"
#include "Data/Octree.h"

Application::Application()
{
    init();
}

void Application::init()
{
    Octree<Render>::createRoot([](const Octree<Render> &, const Render &)->MipResult{});
    _renderer.init();
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
        _renderer.draw();
    }
    engine.stop();
}
