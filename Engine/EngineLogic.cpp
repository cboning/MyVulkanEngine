#include "EngineLogic.h"
#include "../Vkbase/Window.h"
#include "PhysicalSystem.h"
#include "Entity/Cube.h"
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
        Entity::updateCollisionObjects();
        Entity::entity<Cube>("1").checkCollisionWithObject(Entity::entity<Cube>("2"));
        Entity::entity<Cube>("2").checkCollisionWithObject(Entity::entity<Cube>("1"));
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