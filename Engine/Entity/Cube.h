#pragma once
#include "../../Object/Object.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>
#include "Entity.h"

namespace vk
{
    class CommandBuffer;
}

using json = nlohmann::json;

class Camera;

namespace Vkbase
{
    class Buffer;
}

struct alignas(16) CubeUniformBufferData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    alignas(16) glm::vec3 color;
    float _pad = 0.0f;
};

class Cube : public Entity
{
private:
    glm::vec3 _color = glm::vec3(0.0f, 1.0f, 0.0f);

    void init() override;

public:
    Cube(const std::string &name, bool enableGravity = false);
    ~Cube();
    void draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex) const override;
    bool checkCollisionWithObject(const CollisionObject &target);
    bool checkCollisionWithObject(Cube &target);
    void updateUBO(const Camera &_camera, uint32_t index) const;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
};