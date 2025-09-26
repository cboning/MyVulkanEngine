#pragma once
#include <mutex>

class EngineLogic
{
  private:
    int _tick = 0;
    bool _stop = false;
    std::mutex _stopSignalMutex;

    void mainLoop();
  public:
    void run();
    int tick();
    void stop();
};