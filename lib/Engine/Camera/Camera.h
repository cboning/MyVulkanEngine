#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, -5.0f);
    glm::vec3 _front = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
    float _yaw = 90.0f, _pitch = 0.0f;
    glm::mat4 _view = glm::lookAt(_position, _front, _up);
    float _fov = 90.0f;
    float _far = 2000.0f;
    float _near = 0.01f;
    glm::mat4 _perspective;

    glm::vec2 _frameSize;
    bool _light = false;

    void updateView();

public:
    Camera(glm::vec2 frameSize);

    // 移动与旋转
    void movePosTo(float x, float y, float z);
    void moveViewTo(float yaw, float pitch);
    void addPosBy(float x, float y, float z);
    void addViewBy(float yaw, float pitch);
    void moveFront(float distance);
    void moveBack(float distance);
    void moveLeft(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void moveDown(float distance);

    // 视角与投影
    void lookAt(glm::vec3 target);
    void updatePerspective();

    // 设置方法
    void setFrameSize(glm::vec2 size);
    void setLight(bool light);
    void setYaw(float yaw);
    void setPitch(float pitch);
    void setFov(float fov);
    void setNearFar(float nearPlane, float farPlane);

    // 访问方法
    const glm::mat4 &perspective() const;
    const glm::mat4 &view() const;
    const glm::vec3 &position() const;
    const glm::vec3 &front() const;
    float yaw() const;
    float pitch() const;
    float fov() const;
    float nearPlane() const;
    float farPlane() const;
    glm::vec2 frameSize() const;
    bool isLightCamera() const;

    // 重置相机
    void reset(glm::vec3 pos, glm::vec3 target, float fov = 90.0f);
};
