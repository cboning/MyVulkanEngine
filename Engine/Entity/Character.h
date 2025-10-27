#pragma once
#include "../Object/Object.h"
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
    class CommandBuffer;
}

class Character : public Entity
{
private:
    Modelbase::Model &_model;

    void entityInit() override;
    void objectExtraUpdate() override;

public:
    Character(const std::string &name, const std::string &deviceName, const Camera &camera, const Camera &lightCamera, Modelbase::Model &model, bool dynamic = true, const Object &object = {});
    ~Character();
    void onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex, uint32_t frameIndex) const override;
    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) override;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) override;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
    Object &modelObject();
    const Object &modelObject() const;
};