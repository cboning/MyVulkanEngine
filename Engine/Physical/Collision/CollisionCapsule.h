#pragma once
#include "CollisionObject.h"

class CollisionCapsule : public CollisionObject
{
private:
    float _height = 0.0f;
    float _radius = 0.0f;
    glm::vec3 _direction = glm::vec3(0.0f, 1.0f, 0.0f);

public:
    CollisionCapsule();
    CollisionCapsule(const glm::vec3 &center, float height, float radius, const glm::vec3 &direction);

    float height() const;
    float radius() const;
    const glm::vec3 &direction() const;

    void setHeight(float height);
    void setRadius(float radius);
    void setDirection(const glm::vec3 &direction);
    void updateWithObject(const Object &object) override;
};