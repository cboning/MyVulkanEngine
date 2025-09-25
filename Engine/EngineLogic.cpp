#include "EngineLogic.h"

void EngineLogic::run() {}

void EngineLogic::mainLoop()
{
    while (!_stop)
    {
        
        ++_tick;
    }
}

int EngineLogic::tick() { return _tick; }