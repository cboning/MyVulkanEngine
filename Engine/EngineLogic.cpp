#include "EngineLogic.h"
#include "../Vkbase/Window.h"
#include "Entity/Cube.h"
#include "PhysicalSystem.h"
#include <thread>

void EngineLogic::run()
{
    std::thread mainLoopThread(&EngineLogic::mainLoop, this);
    mainLoopThread.detach();
}

void EngineLogic::mainLoop()
{
    std::unique_lock<std::mutex> lock(_stopSignalMutex);
    while (!_stop)
    {
        _physicalSystem.nextTick();
        ++_tick;
    }
}

int EngineLogic::tick() { return _tick; }

void EngineLogic::stop()
{
    _stop = true;
    std::unique_lock<std::mutex> lock(_stopSignalMutex);
}