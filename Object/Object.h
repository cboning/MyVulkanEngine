#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Object
{
public:
    Object();
    Object(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale);
    const glm::mat4 &matModel() const;

    const glm::vec3 &position() const;
    const glm::quat &rotation() const;
    const glm::vec3 &scale() const;
    void setPositon(const glm::vec3 &position);
    void setRotation(const glm::quat &rotation);
    void setScale(const glm::vec3 &scale);

private:
    glm::vec3 _position;
    glm::quat _rotation;
    glm::vec3 _scale;
    glm::mat4 _model;

    void updateMatModel();
};