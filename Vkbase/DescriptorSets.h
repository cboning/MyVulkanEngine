#pragma once
#include "ResourceBase.h"
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class Device;
class DescriptorSets : public ResourceBase
{
    friend class ResourceManager;

  private:
    const Device &_device;
    vk::DescriptorPool _descriptorPool;
    std::unordered_map<std::string, std::vector<vk::DescriptorSet>> _descriptorSets;
    std::unordered_map<std::string, vk::DescriptorSetLayout> _descriptorSetLayouts;
    std::vector<vk::DescriptorSetLayout> _layouts; // record to destroy
    std::unordered_map<std::string, std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>> _descriptorSetLayoutInfos;
    std::unordered_map<vk::DescriptorType, uint32_t> _descriptorPoolSizeInfo;
    std::unordered_map<std::string, uint32_t> _descriptorSetsCounts;

    bool _inited = false;

    DescriptorSets(const std::string &resourceName, const std::string &deviceName);
    ~DescriptorSets();
    void createPool();
    void allocateSets();

  public:
    const std::string addDescriptorSetCreateConfig(std::string name, std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> descriptorTypes,
                                                   uint32_t count, const std::pair<const DescriptorSets *, std::string> &layout = {nullptr, ""});
    void writeSets(const std::string &name, uint32_t binding, const std::vector<vk::DescriptorBufferInfo> &bufferInfos,
                   const std::vector<vk::DescriptorImageInfo> &imageInfos, uint32_t count) const;
    const std::vector<vk::DescriptorSet> &sets(const std::string &name) const;
    const vk::DescriptorSetLayout &layout(const std::string &name) const;
    void init();
};
}; // namespace Vkbase