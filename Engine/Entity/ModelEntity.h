#pragma once
#include "../Object/Object.h"
#include "../Modelbase/Modelbase.h"
#include "Entity.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>

using json = nlohmann::json;

class Camera;

namespace Vkbase
{
class Buffer;
class CommandBuffer;
} // namespace Vkbase

class ModelEntity : public Entity
{
private:
    Modelbase::Model &_model;

    void entityInit() override;
    void objectExtraUpdate() override;

public:
    ModelEntity(const std::string &name, const std::string &deviceName, const Camera &camera, Modelbase::Model &model, bool dynamic = true,
                const Object &object = {});
    ~ModelEntity();
    void onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                uint32_t frameIndex) const override;
    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) override;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) override;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
    Object &modelObject();
    const Object &modelObject() const;
};