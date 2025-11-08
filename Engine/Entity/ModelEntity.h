#pragma once
#include "../Object/Object.h"
#include "../Modelbase/Modelbase.h"
#include "Entity.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>

using json = nlohmann::json;

class Camera;

class ModelEntity : public Entity
{
private:
    Modelbase::Model &_model;
    std::string _instanceName;

    void entityInit() override;
    void objectExtraUpdate() override;

public:
    ModelEntity(const std::string &name, const std::string &deviceName, const Camera &camera, Modelbase::Model &model, bool dynamic = true,
                const Object &object = {});
    ~ModelEntity();
    void onDraw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                uint32_t frameIndex) const override;
    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets) override;
    void writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets) override;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
    Object &modelObject();
    const Object &modelObject() const;
};