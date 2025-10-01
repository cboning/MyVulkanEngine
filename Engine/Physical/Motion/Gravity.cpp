#include "Gravity.h"
#include "../../Entity/Entity.h"


Gravity::Gravity() {}

Gravity::~Gravity() {}

void Gravity::update(float deltaTime)
{
    glm::vec3 &acceleration = entity().acceleration();
    acceleration.y -= 9.8f;
}