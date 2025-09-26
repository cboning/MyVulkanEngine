#include "EngineLogic.h"
#include <thread>

void EngineLogic::run() {
    std::thread mainLoopThread(&EngineLogic::mainLoop, this);
    mainLoopThread.join();
}

void EngineLogic::mainLoop()
{
    std::unique_lock<std::mutex> lock(_stopSignalMutex, std::defer_lock);
    while (!_stop)
    {

        ++_tick;
    }
}

int EngineLogic::tick() { return _tick; }

void EngineLogic::stop() {
    _stop = true;
    std::unique_lock<std::mutex> lock(_stopSignalMutex);
}