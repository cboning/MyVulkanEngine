#pragma once
#include "../../Object/Object.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>
#include "Entity.h"
#include "../Modelbase/Modelbase.h"

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

class ModelEntity : public Entity
{
private:
    Modelbase::Model _model;

    void init() override;
    void objectExtraUpdate() override;

public:
    ModelEntity(const std::string &name, bool dynamic = true, const Object &object = {}, const json &config = {});
    ~ModelEntity();
    void draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex, const std::string &pipelineName, const std::string &uboName) const override;
    void updateUBO(const Camera &camera, uint32_t index, const glm::mat4 &mat, const std::string &uboName) const override;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
    Object &modelObject();
    const Object &modelObject() const;
};