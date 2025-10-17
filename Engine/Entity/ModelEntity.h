#pragma once
#include "../../Object/Object.h"
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
    Modelbase::Model _model;

    void entityInit() override;
    void objectExtraUpdate() override;

public:
    ModelEntity(const std::string &name, const std::string &deviceName, const Camera &camera, bool dynamic = true, const Object &object = {},
                const json &config = {});
    ~ModelEntity();
    void onDraw(Vkbase::CommandBuffer *pCommandBuffer, uint32_t frameIndex, const std::vector<std::any> &args) const override;
    void onUpdateUBO(uint32_t frameIndex, const std::vector<std::any> &) const override;
    void addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) override;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) override;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
    Object &modelObject();
    const Object &modelObject() const;
};