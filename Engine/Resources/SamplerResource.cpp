#include "SamplerResource.h"
#include "../Vkbase/Sampler.h"

namespace Resources
{
const vk::Sampler &SamplerResource::sampler() const { return _pSampler->sampler(); }

SamplerResource::SamplerResource(const std::string &deviceName, const vk::SamplerCreateInfo &info)
    : ResourceBase(ResourceType::Sampler, getNameByArgument(deviceName, info))
{
    _pSampler = std::make_shared<Sampler>(deviceName, info);
    _pSampler->setDestroyCallback([&]() { ResourceManager::instance().removeResource(this); });
}

SamplerResource::~SamplerResource() {}

std::string SamplerResource::getNameByArgument(const std::string &deviceName, const vk::SamplerCreateInfo &info)
{
    return deviceName + "_" + std::to_string(static_cast<int>(info.magFilter)) + "x" + std::to_string(static_cast<int>(info.minFilter)) + "_" +
           std::to_string(static_cast<int>(info.addressModeU)) + std::to_string(static_cast<int>(info.addressModeV)) +
           std::to_string(static_cast<int>(info.addressModeW));
}

SamplerResource::Sampler::Sampler(const std::string &deviceName, const vk::SamplerCreateInfo &info)
    : _sampler(createResource<Vkbase::Sampler>("", deviceName, info))
{
}

const vk::Sampler &SamplerResource::Sampler::sampler()
{
    if (auto p = _sampler.lock<Vkbase::Sampler>())
        return p->sampler();
    return nullptr;
}

} // namespace Resources