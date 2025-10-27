#include "Application.h"
#include "Engine/Modelbase/Modelbase.h"
#include "Data.h"
#include "Engine/EngineLogic.h"
#include "Engine/Data/Octree.h"

Application::Application()
    : _renderer("mainWindow")
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
        _renderer.draw();
    }
    engine.stop();
}
