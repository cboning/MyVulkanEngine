#include "Application.h"
#include "Modelbase/Modelbase.h"
#include "Data.h"

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
