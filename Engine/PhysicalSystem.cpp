#include "PhysicalSystem.h"
#include "Entity/Entity.h"
#include <chrono>

void PhysicalSystem::nextTick()
{
    calcTime();
    Entity::updateAll(_deltaTime);
    Entity::cleanAllCollisionResults();
}

void PhysicalSystem::calcTime()
{
    float currentTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    if (!_lastTime)
        _lastTime = currentTime;

    _deltaTime = currentTime - _lastTime;
    _lastTime = currentTime;
}