#pragma once
#include "CollisionObject.h"
#include <glm/glm.hpp>

class CollisionTriangle : public CollisionObject
{
    friend class CollisionSystem;

private:
    glm::mat3 _vertices;

public:
    CollisionTriangle();
    CollisionTriangle(const glm::vec3 &center, const glm::mat3 &vertices);
    CollisionTriangle(const glm::vec3 &vertex1, const glm::vec3 &vertex2, const glm::vec3 &vertex3);
    glm::mat3 vertices() const;
    glm::vec3 vertex(uint8_t index) const;
    void setVertices(const glm::mat3 &vertices);
    void setVertices(const glm::vec3 &vertex1, const glm::vec3 &vertex2, const glm::vec3 &vertex3);
    void updateWithObject(const Object &object) override;
};