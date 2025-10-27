#include "Sampler.h"
#include "Device.h"

namespace Vkbase
{
Sampler::Sampler(const std::string &resourceName, const std::string &deviceName, const vk::SamplerCreateInfo &createInfo)
    : VkGpuResourceBase(Vkbase::VkResourceType::Sampler, resourceName,
                        *dynamic_cast<Device *>(resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)))
{
    createSampler(createInfo);
}

Sampler::~Sampler()
{
    auto device = _device.device();
    auto sampler = _sampler;
    _onDelayDestroy = [device, sampler]() mutable { device.destroy(sampler); };
}

void Sampler::createSampler(vk::SamplerCreateInfo createInfo)
{
    createInfo.setMaxAnisotropy(_device.physicalDevice().getProperties().limits.maxSamplerAnisotropy);
    _sampler = _device.device().createSampler(createInfo);
}

vk::SamplerCreateInfo Sampler::getDefaultCreateInfo()
{
    return vk::SamplerCreateInfo()
        .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
        .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
        .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
        .setAnisotropyEnable(vk::False)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setMipmapMode(vk::SamplerMipmapMode::eNearest);
}

const vk::Sampler &Sampler::sampler() const { return _sampler; }
} // namespace Vkbase