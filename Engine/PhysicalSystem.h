#pragma once
class PhysicalSystem
{
private:
    float _deltaTime;
    float _lastTime = 0.0f;

    void calcTime();
public:
    void nextTick();
};