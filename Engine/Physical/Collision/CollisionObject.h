#pragma once
#include <glm/glm.hpp>
#include <vector>

enum class CollisionObjectType
{
    Box,
    Sphere
};
struct CollisionResult {
    bool intersect = false;     // 是否相交
    glm::vec3 axis = {0,0,0};   // MTV 的方向 (法线)
    float depth = 0.0f;         // 穿透深度
};

class Object;
class CollisionObject
{
private:
    glm::vec3 _boundBoxSize;
    glm::vec3 _position;
    glm::vec3 _positionInBoundBox;
    glm::mat3 _axes;
    CollisionObjectType _type;

protected:
public:
    CollisionObject(CollisionObjectType type);
    virtual CollisionResult performCollisionDetection(const CollisionObject &target) const = 0;
    glm::vec3 boundBoxSize() const;
    glm::vec3 center() const;
    glm::vec3 halfSize() const;
    glm::vec3 position() const;
    glm::vec3 positionInBoundBox() const;
    glm::mat3 axes() const;
    CollisionObjectType type() const;
    void updateWithObject(const Object &object);
    void setPositionInBoundBox(const glm::vec3 &positionInBoundBox);
};
