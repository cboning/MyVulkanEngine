#include "CollisionObject.h"

glm::vec3 CollisionObject::boundBoxSize() const { return _boundBoxSize; }
glm::vec3 CollisionObject::position() const { return _position; }

glm::vec3 CollisionObject::positionInBoundBox() const { return _positionInBoundBox; }

glm::mat3 CollisionObject::axes() const { return _axes; }

CollisionObjectType CollisionObject::type() const { return _type; }

glm::vec3 CollisionObject::center() const { return _position - _boundBoxSize * _positionInBoundBox + halfSize(); }

glm::vec3 CollisionObject::halfSize() const { return _boundBoxSize * 0.5f; }