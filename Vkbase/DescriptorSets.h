#pragma once
#include "GpuResourceBase.h"
#include <json.hpp>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

using json = nlohmann::json;

namespace Vkbase
{
class Device;
class Image;
class Buffer;
class DescriptorSets : public GpuResourceBase
{
    friend class ResourceManager;
    friend class CommandBuffer;

private:
    struct DescriptorSetsResource
    {
        Image *pImage = nullptr;
        Buffer *pBuffer = nullptr;
    };

    vk::DescriptorPool _descriptorPool;
    std::unordered_map<std::string, std::vector<vk::DescriptorSet>> _descriptorSets;
    std::unordered_map<std::string, vk::DescriptorSetLayout> _descriptorSetLayouts;
    std::vector<vk::DescriptorSetLayout> _layouts; // record to destroy
    std::unordered_map<std::string, std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>> _descriptorSetLayoutInfos;
    std::unordered_map<vk::DescriptorType, uint32_t> _descriptorPoolSizeInfo;
    std::unordered_map<std::string, uint32_t> _descriptorSetsCounts;
    std::unordered_map<std::string, std::vector<std::vector<DescriptorSetsResource>>> _descriptorSetResource;

    bool _inited = false;

    DescriptorSets(const std::string &resourceName, const std::string &deviceName);
    ~DescriptorSets();
    void createPool();
    void allocateSets();
    uint32_t getCount(const json &config);
    const std::vector<vk::DescriptorSet> &sets(const std::string &name) const;

public:
    const std::string addDescriptorSetCreateConfig(std::string name, std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> descriptorTypes,
                                                   uint32_t count, const std::pair<const DescriptorSets *, std::string> &layout = {nullptr, ""});
    void writeSets(const std::string &name, uint32_t binding, std::vector<std::pair<vk::DescriptorBufferInfo, Buffer *>> bufferInfos,
                   std::vector<std::pair<vk::DescriptorImageInfo, Image *>> imageInfos, uint32_t count);
    void addDescriptorSetCreateConfigWithJson(const json &config);
    void writeSetsWithJson(const json &config);
    const vk::DescriptorSetLayout &layout(const std::string &name) const;
    void init();
};
}; // namespace Vkbase