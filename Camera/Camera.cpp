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
    _yaw += yaw * 0.3f;
    _pitch += pitch * 0.3f;
    updateView();
}

void Camera::updateView()
{
    if (_pitch > 89.9f)
        _pitch = 89.9f;
    else if (_pitch < -89.9f)
        _pitch = -89.9f;

    _yaw = fmod(_yaw, 360.0f);
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

void Camera::lookAt(glm::vec3 target)
{
    glm::vec3 front = glm::normalize(target - _position);
    _yaw = glm::degrees(atan2(front.z, front.x));
    _pitch = glm::degrees(asin(front.y));
    updateView();
}

void Camera::updatePerspective()
{
    if (_light)
    {
        _perspective = glm::ortho(-_frameSize.x / 2, _frameSize.x / 2, -_frameSize.y / 2, _frameSize.y / 2, _near, _far);
        _perspective[1][1] *= -1; // Vulkan Y 翻转
        return;
    }
    _perspective = glm::perspective(glm::radians(_fov), _frameSize.x / _frameSize.y, _near, _far);
    // _perspective[1][1] *= -1; // Vulkan Y 翻转
}

void Camera::setFrameSize(glm::vec2 size) { _frameSize = size; }
void Camera::setLight(bool light) { _light = light; }
void Camera::setYaw(float yaw)
{
    _yaw = yaw;
    updateView();
}
void Camera::setPitch(float pitch)
{
    _pitch = pitch;
    updateView();
}
void Camera::setFov(float fov)
{
    _fov = fov;
    updatePerspective();
}
void Camera::setNearFar(float nearPlane, float farPlane)
{
    _near = nearPlane;
    _far = farPlane;
    updatePerspective();
}

const glm::mat4 &Camera::view() const { return _view; }
const glm::mat4 &Camera::perspective() const { return _perspective; }
const glm::vec3 &Camera::position() const { return _position; }
const glm::vec3 &Camera::front() const { return _front; }
float Camera::yaw() const { return _yaw; }
float Camera::pitch() const { return _pitch; }
float Camera::fov() const { return _fov; }
float Camera::nearPlane() const { return _near; }
float Camera::farPlane() const { return _far; }
glm::vec2 Camera::frameSize() const { return _frameSize; }
bool Camera::isLightCamera() const { return _light; }

void Camera::reset(glm::vec3 pos, glm::vec3 target, float fov)
{
    _position = pos;
    lookAt(target);
    _fov = fov;
    updatePerspective();
}
