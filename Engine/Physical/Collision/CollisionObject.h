#pragma once
#include <glm/glm.hpp>

class CollisionObject
{
  private:
    glm::vec3 _size;
    glm::vec3 _centerPosition;

  public:
    bool performCollisionDetection(CollisionObject target);
};