#include "Camera.h"
#include <iostream>

Camera::Camera(glm::vec2 frameSize) : _frameSize(frameSize) {}

void Camera::movePosTo(float x, float y, float z)
{
    _position = glm::vec3(x, y, z);
    updateView();
}

void Camera::moveViewTo(float yaw, float pitch)
{
    _yaw = yaw;
    _pitch = pitch;
    updateView();
}

void Camera::addPosBy(float x, float y, float z)
{
    _position += glm::vec3(x, y, z);
    updateView();
}

void Camera::addViewBy(float yaw, float pitch)
{
    _yaw += yaw * 0.3;
    _pitch += pitch * 0.3;
    updateView();
}

void Camera::updateView()
{
    if (_pitch > 89.9)
        _pitch = 89.9;
    else if (_pitch < -89.9)
        _pitch = -89.9;
    _yaw = fmod(_yaw, 360);
    double yaw = glm::radians(_yaw), pitch = glm::radians(_pitch);
    _front.x = cos(yaw) * cos(pitch);
    _front.y = sin(pitch);
    _front.z = sin(yaw) * cos(pitch);

    _front = glm::normalize(_front);

    _view = glm::lookAt(_position, _position + _front, _up);
}

void Camera::moveFront(float distance)
{
    glm::vec3 front = glm::vec3(cos(glm::radians(_yaw)), 0.0f, sin(glm::radians(_yaw)));
    _position += front * distance;
    updateView();
}

void Camera::moveBack(float distance)
{
    glm::vec3 back = glm::vec3(-cos(glm::radians(_yaw)), 0.0f, -sin(glm::radians(_yaw)));
    _position += back * distance;
    updateView();
}

void Camera::moveLeft(float distance)
{
    glm::vec3 left = glm::vec3(sin(glm::radians(_yaw)), 0.0f, -cos(glm::radians(_yaw)));
    _position += left * distance;
    updateView();
}

void Camera::moveRight(float distance)
{
    glm::vec3 right = glm::vec3(-sin(glm::radians(_yaw)), 0.0f, cos(glm::radians(_yaw)));
    _position += right * distance;
    updateView();
}

void Camera::moveUp(float distance)
{
    _position.y += distance;
    updateView();
}

void Camera::moveDown(float distance)
{
    _position.y -= distance;
    updateView();
}

const glm::mat4 &Camera::view() const { return _view; }

const glm::vec3 &Camera::position() const { return _position; }

const glm::vec3 &Camera::front() const { return _front; }

void Camera::updatePerspective()
{
    _perspective = glm::perspective(glm::radians(_fov), _frameSize.x / _frameSize.y, _near, _far);
    _perspective[1][1] *= -1;
}

const glm::mat4 &Camera::perspective() const { return _perspective; }

void Camera::setFrameSize(glm::vec2 size) { _frameSize = size; }