#pragma once
#include "ResourceBase.h"
#include <vulkan/vulkan.hpp>
namespace Vkbase
{
class Device;
class Sampler : public ResourceBase
{
    friend class ResourceManager;

  private:
    vk::Sampler _sampler;
    const Device &_device;
    void createSampler(vk::SamplerCreateInfo createInfo);
    Sampler(const std::string &resourceName, const std::string &deviceName, const vk::SamplerCreateInfo &createInfo = getDefaultCreateInfo());
    ~Sampler();

  public:
    const vk::Sampler &sampler() const;
    static vk::SamplerCreateInfo getDefaultCreateInfo();
};
} // namespace Vkbase