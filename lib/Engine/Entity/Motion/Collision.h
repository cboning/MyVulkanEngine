#pragma once

#include "Motion.h"
class Entity;

class Collision : Motion
{
private:
public:
    Collision();
    ~Collision() override;
    void update(float deltaTime) override;
};