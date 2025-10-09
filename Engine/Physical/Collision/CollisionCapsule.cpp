#include "CollisionCapsule.h"
#include "../../../Object/Object.h"

CollisionCapsule::CollisionCapsule() : CollisionObject(CollisionObjectType::Capsule) {}

CollisionCapsule::CollisionCapsule(const glm::vec3 &center, float height, float radius, const glm::vec3 &direction)
    : CollisionObject(CollisionObjectType::Capsule, center), _height(height), _radius(radius), _direction(direction)
{
}

float CollisionCapsule::height() const { return _height; }

float CollisionCapsule::radius() const { return _radius; }

const glm::vec3 &CollisionCapsule::direction() const { return _direction; }

void CollisionCapsule::setHeight(float height) { _height = height; }

void CollisionCapsule::setRadius(float radius) { _radius = radius; }

void CollisionCapsule::setDirection(const glm::vec3 &direction) { _direction = direction; }

void CollisionCapsule::updateWithObject(const Object &object)
{
    setCenter(object.position());
    setHeight(object.scale().y);
    setRadius(object.scale().x / 2.0f);
}
