#pragma once
#include "Motion.h"
#include <glm/glm.hpp>
class Entity;

class Push : Motion
{
private:
    glm::vec3 _acceleration = glm::vec3(0.0f);

public:
    Push();
    ~Push() override;
    void update(float deltaTime) override;

    void setAcceleration(const glm::vec3 &acceleration);
    const glm::vec3 &acceleration() const;
};