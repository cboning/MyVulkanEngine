#include "CollisionBox.h"
#include "../../../Object/Object.h"
#include "CollisionObject.h"
#include <glm/glm.hpp>

CollisionBox::CollisionBox() : CollisionObject(CollisionObjectType::Box) {}

CollisionBox::CollisionBox(const glm::vec3 &center, const glm::vec3 &boundBoxSize, const glm::mat3 &axes)
    : CollisionObject(CollisionObjectType::Box, center), _boundBoxSize(boundBoxSize), _axes(axes)
{
}

CollisionBox::CollisionBox(const Object &object)
    : CollisionObject(CollisionObjectType::Box)
{
    updateWithObject(object);
}

const glm::vec3 &CollisionBox::boundBoxSize() const { return _boundBoxSize; }

glm::vec3 CollisionBox::halfSize() const { return _boundBoxSize * 0.5f; }

const glm::mat3 &CollisionBox::axes() const { return _axes; }

void CollisionBox::setBoundBoxSize(const glm::vec3 &boundBoxSize) { _boundBoxSize = boundBoxSize; }

void CollisionBox::setAxes(const glm::mat3 &axes) { _axes = axes; }

void CollisionBox::updateWithObject(const Object &object)
{
    setBoundBoxSize(object.scale());
    setCenter(object.position());
    setAxes(glm::toMat3(object.rotation()));
}

