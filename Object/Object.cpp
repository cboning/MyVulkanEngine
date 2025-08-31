#include "Object.h"
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

Object::Object(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale)
    : _position(position), _rotation(rotation), _scale(scale)
{
    updateMatModel();
}

Object::Object()
    : Object(glm::vec3(), glm::quat(), glm::vec3(1.0f))
{

}

const glm::vec3 &Object::position() const
{
    return _position;
}

const glm::vec3 &Object::scale() const
{
    return _scale;
}

const glm::quat &Object::rotation() const
{
    return _rotation;
}

const glm::mat4 &Object::matModel() const
{
    return _model;
}

void Object::setPositon(const glm::vec3 &position)
{
    _position = position;
    updateMatModel();
}

void Object::setRotation(const glm::quat &rotation)
{
    _rotation = rotation;
    updateMatModel();
}

void Object::setScale(const glm::vec3 &scale)
{
    _scale = scale;
    updateMatModel();
}

void Object::updateMatModel()
{
    _model = glm::translate(glm::scale(glm::toMat4(_rotation), _scale), _position);
}