#include "Collision.h"
#include "../../Entity/Entity.h"
#include "../Collision/CollisionObject.h"
#include "../Collision/CollisionObjectDelegator.h"

Collision::Collision() {}

Collision::~Collision() {}

void Collision::update(float deltaTime)
{
    if (deltaTime < 1e-3f)
        deltaTime = 1e-3f;
    glm::vec3 &velocity = entity().velocity();

    const Entity &cEntity = entity();
    // 当前 tick 的碰撞记录
    const auto &collisions = cEntity.collisionObject()->collisionResults();
    if (collisions.empty())
        return;

    for (const auto &c : collisions)
    {
        if (!c.intersect)
            continue;

        if (c.axis == glm::vec3())
            continue;
            
        glm::vec3 n = -glm::normalize(c.axis);
        float an = glm::dot(velocity, n);

        // 如果加速度朝向静态物体（试图钻进去）
        if (an < 0.0f)
        {
            velocity -= 1.5f * an * n;
        }

        an = glm::dot(dynamic_cast<const CollisionObjectDelegator *>(c.pTarget)->entity()->velocity(), n);
        if (an > 0.0f)
        {
            velocity += 0.5f * an * n;
        }
    }
}
