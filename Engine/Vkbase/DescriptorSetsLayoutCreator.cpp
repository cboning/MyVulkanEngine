#include "DescriptorSetsLayoutCreator.h"
namespace Vkbase
{
DescriptorSetsLayoutCreator::DescriptorSetsLayoutCreator(const vk::Device &device) : _device(device) {}

vk::DescriptorSetLayout DescriptorSetsLayoutCreator::createLayout(const DescriptorKey &key) const
{
    uint32_t bindingCount = 0;
    std::vector<vk::DescriptorSetLayoutBinding> bindings(key.size());
    for (const DescriptorKey::value_type &descriptorType : key)
        bindings[bindingCount]
            .setDescriptorCount(1)
            .setDescriptorType(descriptorType.first)
            .setPImmutableSamplers(nullptr)
            .setBinding(bindingCount++)
            .setStageFlags(descriptorType.second);
    vk::DescriptorSetLayoutCreateInfo info;
    info.setBindings(bindings);

    return _device.createDescriptorSetLayout(info);
}

DescriptorSetsLayoutCreator::~DescriptorSetsLayoutCreator() {}

vk::DescriptorSetLayout DescriptorSetsLayoutCreator::get(const DescriptorKey &key)
{
    try
    {
        return _layouts.at(key);
    }
    catch (std::out_of_range e)
    {
        _layouts.insert({key, createLayout(key)});
    }
}

} // namespace Vkbase