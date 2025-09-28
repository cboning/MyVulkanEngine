#include "Gravity.h"
#include "../../Entity/Entity.h"

Gravity::Gravity(Entity &entity) : Motion(entity) {}

Gravity::~Gravity() {}

void Gravity::update(float deltaTime)
{
    glm::vec3 &acceleration = entity().acceleration();
    if (acceleration.y > 0.0f)
        return;
    acceleration.y -= 9.8f;
}