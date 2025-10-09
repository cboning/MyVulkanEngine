#pragma once
#include <glm/glm.hpp>

enum class CollisionObjectType
{
    Box,
    Capsule,
    Triangle
};

class Object;

class CollisionObject
{
    friend class CollisionSystem;

private:
    const CollisionObjectType _type;
    glm::vec3 _center{0.0f};

protected:
    CollisionObject(CollisionObjectType type, const glm::vec3 &center);
    CollisionObject(CollisionObjectType type);
    virtual ~CollisionObject() = default;

public:
    CollisionObject(const CollisionObject &) = delete;
    CollisionObject &operator=(const CollisionObject &) = delete;

    const glm::vec3 &center() const;
    CollisionObjectType type() const;

    void setCenter(const glm::vec3 &center);

    virtual void updateWithObject(const Object &object) = 0;
};
