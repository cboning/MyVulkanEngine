#include "Friction.h"
#include "../../Entity/Entity.h"


Friction::Friction() {}

Friction::~Friction() {}

void Friction::update(float deltaTime)
{
    glm::vec3 &acceleration = entity().acceleration();
    glm::vec3 &velocity = entity().velocity();
    if (glm::length(velocity) < 1e-3f)
        return;
    acceleration -= 1.5f * velocity;
}