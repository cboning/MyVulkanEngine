#pragma once
#include "GpuResourceBase.h"
#include <vulkan/vulkan.hpp>
namespace Vkbase
{
class Device;
class Sampler : public GpuResourceBase
{
    friend class ResourceManager;

private:
    vk::Sampler _sampler;
    void createSampler(vk::SamplerCreateInfo createInfo);
    Sampler(const std::string &resourceName, const std::string &deviceName, const vk::SamplerCreateInfo &createInfo = getDefaultCreateInfo());
    ~Sampler();

public:
    const vk::Sampler &sampler() const;
    static vk::SamplerCreateInfo getDefaultCreateInfo();
};
} // namespace Vkbase