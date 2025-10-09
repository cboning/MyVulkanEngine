#include "PhysicalSystem.h"
#include "Entity/Entity.h"
#include "Physical/Collision/CollisionSystem.h"
#include <chrono>
#include <thread>

void PhysicalSystem::nextTick()
{
    calcTime();

    Entity::updateCollisionObjects();
    CollisionSystem::instance().update();
    Entity::updateAll(_deltaTime);
}

void PhysicalSystem::calcTime()
{
    double currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    if (!_lastTime)
        _lastTime = currentTime;

    std::this_thread::sleep_for(std::chrono::milliseconds((int)((1.0f / 300.0f - currentTime + _lastTime))));
    currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    _deltaTime = currentTime - _lastTime;
    _lastTime = currentTime;
}