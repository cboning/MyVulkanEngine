#include "DescriptorSets.h"
#include "Device.h"
namespace Vkbase
{
    DescriptorSets::DescriptorSets(const std::string &resourceName, const std::string &deviceName)
        : ResourceBase(Vkbase::ResourceType::DescriptorSets, resourceName), _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName))))
    {
    }

    DescriptorSets::~DescriptorSets()
    {
        for (const vk::DescriptorSetLayout &layout : _layouts)
            _device.device().destroyDescriptorSetLayout(layout);
        _descriptorSetLayouts.clear();

        _device.device().destroyDescriptorPool(_descriptorPool);
    }

    void DescriptorSets::createPool()
    {
        std::vector<vk::DescriptorPoolSize> poolSizes(_descriptorPoolSizeInfo.size());
        int i = 0;
        uint32_t maxSets = 0;
        for (std::unordered_map<vk::DescriptorType, uint32_t>::iterator iterator = _descriptorPoolSizeInfo.begin(); iterator != _descriptorPoolSizeInfo.end(); ++iterator)
        {
            poolSizes[i].setType(iterator->first).setDescriptorCount(iterator->second);
            maxSets += iterator->second;
            i++;
        }
        if (!maxSets)
            return;
        vk::DescriptorPoolCreateInfo createInfo;
        createInfo.setPoolSizes(poolSizes)
            .setMaxSets(maxSets);

        _descriptorPool = _device.device().createDescriptorPool(createInfo);
    }

    void DescriptorSets::allocateSets()
    {
        if (!_descriptorPool)
            return ;
        for (const std::pair<const std::string, vk::DescriptorSetLayout> &descriptorSetLayout : _descriptorSetLayouts)
        {
            uint32_t count = _descriptorSetsCounts[descriptorSetLayout.first];
            if (!count)
                continue;
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(count, descriptorSetLayout.second);

            vk::DescriptorSetAllocateInfo allocateInfo;
            allocateInfo.setDescriptorPool(_descriptorPool)
                .setDescriptorSetCount(count)
                .setSetLayouts(descriptorSetLayouts);

            _descriptorSets[descriptorSetLayout.first] = _device.device().allocateDescriptorSets(allocateInfo);
        }
    }

    const std::string DescriptorSets::addDescriptorSetCreateConfig(std::string name, std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> descriptorTypes, uint32_t count, const std::pair<const DescriptorSets*, std::string> &layout)
    {
        if (_descriptorSetLayoutInfos.count(name))
        {
            uint32_t i = 0;
            while (true)
            {
                if (_descriptorSetLayoutInfos.count(name + "_" + std::to_string(i)))
                {
                    name += "_" + std::to_string(i);
                    break;
                }
                i++;
            }
        }

        if (layout.first)
            _descriptorSetLayouts[name] = connectTo(layout.first)->layout(layout.second);
        else
        {
            uint32_t bindingCount = 0;
            std::vector<vk::DescriptorSetLayoutBinding> bindings(descriptorTypes.size());
            for (const std::pair<vk::DescriptorType, vk::ShaderStageFlags> &descriptorType : descriptorTypes)
                bindings[bindingCount].setDescriptorCount(1).setDescriptorType(descriptorType.first).setPImmutableSamplers(nullptr).setBinding(bindingCount++).setStageFlags(descriptorType.second);

            vk::DescriptorSetLayoutCreateInfo createInfo;
            createInfo.setBindings(bindings);
            vk::DescriptorSetLayout layout = _device.device().createDescriptorSetLayout(createInfo);
            _descriptorSetLayouts[name] = layout;
            _layouts.push_back(layout);
        }

        _descriptorSetsCounts[name] = count;
        _descriptorSetLayoutInfos[name] = descriptorTypes;
        if (count)
            for (const std::pair<vk::DescriptorType, vk::ShaderStageFlags> &descriptorType : descriptorTypes)
            {
                if (_descriptorPoolSizeInfo.find(descriptorType.first) == _descriptorPoolSizeInfo.end())
                    _descriptorPoolSizeInfo[descriptorType.first] = 0;
                _descriptorPoolSizeInfo[descriptorType.first] += count;
            }

        return name;
    }

    void DescriptorSets::writeSets(const std::string &name, uint32_t binding, const std::vector<vk::DescriptorBufferInfo> &bufferInfos, const std::vector<vk::DescriptorImageInfo> &imageInfos, uint32_t count) const
    {
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets(count);
        for (int i = 0; i < count; i++)
        {
            writeDescriptorSets[i].setDstBinding(binding).setBufferInfo(bufferInfos[i]).setImageInfo(imageInfos[i]).setDstSet(_descriptorSets.at(name)[i]).setDescriptorType(_descriptorSetLayoutInfos.at(name)[binding].first);
        }
        _device.device().updateDescriptorSets(writeDescriptorSets, nullptr);
    }

    const std::vector<vk::DescriptorSet> &DescriptorSets::sets(const std::string &name) const
    {
        if (!_inited)
            throw std::runtime_error("The descriptor sets have not been inited!");

        std::unordered_map<std::string, std::vector<vk::DescriptorSet>>::const_iterator iter = _descriptorSets.find(name);
        if (iter == _descriptorSets.end())
            throw std::runtime_error("Index out of range!");

        return iter->second;
    }

    const vk::DescriptorSetLayout &DescriptorSets::layout(const std::string &name) const
    {
        std::unordered_map<std::string, vk::DescriptorSetLayout>::const_iterator iter = _descriptorSetLayouts.find(name);
        if (iter == _descriptorSetLayouts.end())
            throw std::runtime_error("Index out of range!");

        return iter->second;
    }

    void DescriptorSets::init()
    {
        if (_inited)
            return;
        _inited = true;

        createPool();
        allocateSets();
    }
};