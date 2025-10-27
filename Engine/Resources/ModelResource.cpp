#include "ModelResource.h"
#include "../Modelbase/Model.h"
#include "SamplerResource.h"
namespace Resources
{
Modelbase::Model *ModelResource::model() const { return _pModel.get(); }

ModelResource::ModelResource(const std::string &deviceName, const vk::SamplerCreateInfo &info, const json &config)
    : ResourceBase(ResourceType::Model, getNameByArgument(deviceName, info, config))
{
    _pModel = std::make_shared<Modelbase::Model>(deviceName, ResourceManager::instance().getResource<SamplerResource>(deviceName, info).sampler(), config);
    _pModel->setDestroyCallback([&]() { ResourceManager::instance().removeResource(this); });
}

ModelResource::~ModelResource() {}

std::string ModelResource::getNameByArgument(const std::string &deviceName, const vk::SamplerCreateInfo &info, const json &config)
{
    return deviceName + "_" + std::to_string(static_cast<int>(info.magFilter)) + "x" + std::to_string(static_cast<int>(info.minFilter)) + "_" +
           std::to_string(static_cast<int>(info.addressModeU)) + std::to_string(static_cast<int>(info.addressModeV)) +
           std::to_string(static_cast<int>(info.addressModeW)) + config.dump();
}

} // namespace Resources