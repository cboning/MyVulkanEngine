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
    glm::mat4 lightSpaceMatrix;
    glm::vec3 color;
    float _pad = 0.0f;
};

class CubeOutline : public Entity
{
private:
    glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f);
    std::vector<Vkbase::Buffer *> _ubos;

    void init() override;
    void objectExtraUpdate() override {}

public:
    CubeOutline(const std::string &name, const Object &object = {});
    ~CubeOutline();

    void draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex, const std::string &pipelineName,
              const std::string &uboName) const override;

    void updateUBO(const Camera &camera, uint32_t index, const glm::mat4 &mat, const std::string &uboName) const override;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
};
