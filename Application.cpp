#include "Application.h"
#include "Modelbase/Modelbase.h"
#include "Data.h"
#include "Engine/EngineLogic.h"

Application::Application()
{
    init();
}

void Application::init()
{
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
