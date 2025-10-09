#pragma once
#include "Motion.h"
class Entity;

class Friction : Motion
{
private:
public:
    Friction();
    ~Friction() override;
    void update(float deltaTime) override;
};