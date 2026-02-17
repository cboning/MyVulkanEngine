#pragma once
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
using DescriptorKey = std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>;
}

namespace std
{
template <> struct hash<Vkbase::DescriptorKey>
{
    size_t operator()(const Vkbase::DescriptorKey &key) const noexcept
    {
        size_t h = 0;

        for (auto const &[type, stage] : key)
        {
            size_t th = std::hash<int>{}(static_cast<int>(type));
            size_t sh = std::hash<uint32_t>{}(static_cast<uint32_t>(stage));

            h ^= th + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            h ^= sh + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};
} // namespace std

namespace Vkbase
{
using DescriptorKey = std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>;
class DescriptorSetsLayoutCreator
{
    friend class Device;

private:
    vk::Device _device;
    std::unordered_map<DescriptorKey, vk::DescriptorSetLayout> _layouts;

    DescriptorSetsLayoutCreator(const vk::Device &device);
    vk::DescriptorSetLayout createLayout(const DescriptorKey &key) const;

public:
    ~DescriptorSetsLayoutCreator();
    vk::DescriptorSetLayout get(const DescriptorKey &key);
};
} // namespace Vkbase
