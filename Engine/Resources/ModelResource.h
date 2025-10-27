#pragma once
#include <json.hpp>
#include "ResourceBase.h"
#include <vulkan/vulkan.hpp>
namespace Modelbase
{
class Model;
}
using json = nlohmann::json;
namespace Resources
{
class ModelResource : public ResourceBase
{
    friend class ResourceManager;

public:
    Modelbase::Model *model() const;

private:
    std::shared_ptr<Modelbase::Model> _pModel;

    ModelResource(const std::string &deviceName, const vk::SamplerCreateInfo &info, const json &config);
    ~ModelResource() override;

    static std::string getNameByArgument(const std::string &deviceName, const vk::SamplerCreateInfo &info, const json &config);
};
} // namespace Resources
