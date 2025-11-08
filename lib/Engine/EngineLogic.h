#pragma once
#include "PhysicalSystem.h"
#include <mutex>

class EngineLogic
{
private:
    int _tick = 0;
    bool _stop = false;
    std::mutex _stopSignalMutex;
    PhysicalSystem _physicalSystem;

    void mainLoop();

public:
    void run();
    int tick();
    void stop();
};