#pragma once

class EngineLogic
{
  private:
    int _tick = 0;
    bool _stop = false;

    void mainLoop();
  public:
    void run();
    int tick();
};