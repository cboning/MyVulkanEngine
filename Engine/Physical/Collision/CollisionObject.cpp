#include "CollisionObject.h"
#include "../Object/Object.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

CollisionObject::CollisionObject(CollisionObjectType type) : _type(type) {}

glm::vec3 CollisionObject::boundBoxSize() const { return _boundBoxSize; }
glm::vec3 CollisionObject::position() const { return _position; }

glm::vec3 CollisionObject::positionInBoundBox() const { return _positionInBoundBox; }

glm::mat3 CollisionObject::axes() const { return _axes; }

CollisionObjectType CollisionObject::type() const { return _type; }

glm::vec3 CollisionObject::center() const { return _position + _axes * (halfSize() - _boundBoxSize * _positionInBoundBox); }

glm::vec3 CollisionObject::halfSize() const { return _boundBoxSize * 0.5f; }

void CollisionObject::updateWithObject(const Object &object)
{
    _axes = glm::toMat3(object.rotation());
    _boundBoxSize = object.scale();
    _position = object.position();
}

void CollisionObject::setPositionInBoundBox(const glm::vec3 &positionInBoundBox)
{
    _positionInBoundBox = positionInBoundBox;
}