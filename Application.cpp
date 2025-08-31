#include "Application.h"
#include "Modelbase/Modelbase.h"
#include "Data.h"
#define SPEED 10.0f

Application::Application()
{
#ifdef DEBUG
    init();
#endif
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
    while (Render::shouldEndApplication())
    {
        _renderer.draw();
    }
}
