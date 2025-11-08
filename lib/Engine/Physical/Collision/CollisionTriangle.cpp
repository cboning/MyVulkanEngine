#include "CollisionTriangle.h"
#include "../../Object/Object.h"

CollisionTriangle::CollisionTriangle() : CollisionObject(CollisionObjectType::Triangle) {}

CollisionTriangle::CollisionTriangle(const glm::vec3 &center, const glm::mat3 &vertexs) : CollisionObject(CollisionObjectType::Triangle, center), _vertices(vertexs) {}

CollisionTriangle::CollisionTriangle(const glm::vec3 &vertex1, const glm::vec3 &vertex2, const glm::vec3 &vertex3)
    : CollisionObject(CollisionObjectType::Triangle), _vertices(glm::mat3(vertex1, vertex2, vertex3))
{
}

glm::mat3 CollisionTriangle::vertices() const { return _vertices + glm::mat3(center(), center(), center()); }

glm::vec3 CollisionTriangle::vertex(uint8_t index) const { return _vertices[index] + center(); }

void CollisionTriangle::setVertices(const glm::mat3 &vertexs) { _vertices = vertexs; }

void CollisionTriangle::setVertices(const glm::vec3 &vertex1, const glm::vec3 &vertex2, const glm::vec3 &vertex3) { _vertices = glm::mat3(vertex1, vertex2, vertex3); }

void CollisionTriangle::updateWithObject(const Object &object)
{
    setCenter(object.position());
}
