#include "PhysicalSystem.h"
#include "Entity/Entity.h"
#include <chrono>
#include <thread>

void PhysicalSystem::nextTick()
{
    calcTime();
    Entity::updateAll(_deltaTime);
    Entity::cleanAllCollisionResults();
}

void PhysicalSystem::calcTime()
{
    double currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    if (!_lastTime)
        _lastTime = currentTime;

    std::this_thread::sleep_for(std::chrono::milliseconds((int)((1.0f / 60.0f - currentTime + _lastTime))));
    currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    _deltaTime = currentTime - _lastTime;
    _lastTime = currentTime;
}