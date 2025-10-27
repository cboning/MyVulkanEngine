#pragma once

#include "../Vkbase/VkResourcesDelegator.h"
#include "ResourceBase.h"

namespace Vkbase
{
class Sampler;
}

namespace Resources
{

class SamplerResource : public ResourceBase
{
    friend class ResourceManager;

public:
    const vk::Sampler &sampler() const;

private:
    class Sampler : public Vkbase::VkResourcesDelegator
    {
    public:
        Vkbase::Sampler *_pSampler;
        Sampler(const std::string &deviceName, const vk::SamplerCreateInfo &info);

        const vk::Sampler &sampler();
    };
    std::shared_ptr<Sampler> _pSampler;

    SamplerResource(const std::string &deviceName, const vk::SamplerCreateInfo &info);
    ~SamplerResource() override;

    static std::string getNameByArgument(const std::string &deviceName, const vk::SamplerCreateInfo &info);
};
} // namespace Resources
