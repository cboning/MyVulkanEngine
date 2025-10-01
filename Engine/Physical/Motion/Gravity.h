#pragma once
#include "Motion.h"
class Entity;

class Gravity : Motion
{
private:
public:
    Gravity();
    ~Gravity() override;
    void update(float deltaTime) override;
};