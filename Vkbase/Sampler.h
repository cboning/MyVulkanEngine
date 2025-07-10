#pragma once
#include <vulkan/vulkan.hpp>
#include "ResourceBase.h"
namespace Vkbase
{
    class Device;
    class Sampler : public ResourceBase
    {
    private:
        vk::Sampler _sampler;
        const Device &_device;
        void createSampler(vk::SamplerCreateInfo createInfo);
        
    public:
        Sampler(const std::string &resourceName, const std::string &deviceName, const vk::SamplerCreateInfo &createInfo = getDefaultCreateInfo());
        ~Sampler();
        const vk::Sampler &sampler() const;
        static vk::SamplerCreateInfo getDefaultCreateInfo();

    };
}