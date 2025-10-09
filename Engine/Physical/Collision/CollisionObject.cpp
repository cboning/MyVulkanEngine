#include "CollisionObject.h"
#include "../Object/Object.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

CollisionObject::CollisionObject(CollisionObjectType type, const glm::vec3 &center) : _type(type), _center(center) {}

CollisionObject::CollisionObject(CollisionObjectType type) : _type(type) {}

CollisionObjectType CollisionObject::type() const { return _type; }

void CollisionObject::setCenter(const glm::vec3 &center) { _center = center; }

const glm::vec3 &CollisionObject::center() const { return _center; }