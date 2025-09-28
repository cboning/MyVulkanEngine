#pragma once
class PhysicalSystem
{
private:
    double _deltaTime;
    double _lastTime = 0.0f;

    void calcTime();
public:
    void nextTick();
};