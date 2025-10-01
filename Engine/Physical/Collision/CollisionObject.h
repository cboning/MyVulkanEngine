#pragma once
#include <glm/glm.hpp>


enum class CollisionObjectType
{
    Box,
    Ellipsoid
};

class Object;
// class CollisionSystem;
class CollisionObject
{
    friend class CollisionSystem;

private:
    glm::vec3 _boundBoxSize;
    glm::vec3 _position;
    glm::vec3 _positionInBoundBox;
    glm::mat3 _axes;
    CollisionObjectType _type;

protected:
    CollisionObject(CollisionObjectType type);
    ~CollisionObject();

public:
    glm::vec3 boundBoxSize() const;
    glm::vec3 center() const;
    glm::vec3 halfSize() const;
    const glm::vec3 &position() const;
    const glm::vec3 &positionInBoundBox() const;
    const glm::mat3 &axes() const;
    CollisionObjectType type() const;

    void setBoundBoxSize(glm::vec3 boundBoxSize);
    void setPosition(glm::vec3 position);
    void setAxes(glm::mat3 axes);

    void setPositionInBoundBox(const glm::vec3 &positionInBoundBox);
    void updateWithObject(const Object &object);
};
