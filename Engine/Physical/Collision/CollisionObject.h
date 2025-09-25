#pragma once
#include <glm/glm.hpp>

class CollisionObject
{
  protected:
    glm::vec3 _boundBox;
    glm::vec3 _position;
    glm::vec3 _positionInBoundBox;

  public:
    virtual bool performCollisionDetection(const CollisionObject &target) = 0;
};