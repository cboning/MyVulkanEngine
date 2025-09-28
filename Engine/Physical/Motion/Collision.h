#pragma once

#include "Motion.h"
class Entity;

class Collision : Motion
{
private:
public:
    Collision(Entity &entity);
    ~Collision() override;
    void update(float deltaTime) override;
};