#include "DescriptorSets.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "../Resources/SamplerResource.h"
#include "Buffer.h"
#include "Device.h"
#include "Image.h"
#include "Sampler.h"
#include "Swapchain.h"
namespace Vkbase
{
DescriptorSets::DescriptorSets(const std::string &resourceName, const std::string &deviceName)
    : VkGpuResourceBase(Vkbase::VkResourceType::DescriptorSets, resourceName, resourceManager().resource(Vkbase::VkResourceType::Device, deviceName))
{
}

DescriptorSets::~DescriptorSets()
{
    vk::Device device;
    if (auto p = _device.lock<Device>())
        device = p->device();
    auto layouts = _layouts;
    auto descriptorPool = _descriptorPool;

    _onDelayDestroy = [device, layouts, descriptorPool]()
    {
        for (const vk::DescriptorSetLayout &layout : layouts)
            device.destroyDescriptorSetLayout(layout);
        device.destroyDescriptorPool(descriptorPool);
    };
}

void DescriptorSets::createPool()
{
    std::vector<vk::DescriptorPoolSize> poolSizes(_descriptorPoolSizeInfo.size());
    int i = 0;
    uint32_t maxSets = 0;
    for (std::unordered_map<vk::DescriptorType, uint32_t>::iterator iterator = _descriptorPoolSizeInfo.begin(); iterator != _descriptorPoolSizeInfo.end();
         ++iterator)
    {
        poolSizes[i].setType(iterator->first).setDescriptorCount(iterator->second);
        maxSets += iterator->second;
        i++;
    }
    if (!maxSets)
        return;
    vk::DescriptorPoolCreateInfo createInfo;
    createInfo.setPoolSizes(poolSizes).setMaxSets(maxSets);

    if (auto p = _device.lock<Device>())
        _descriptorPool = p->device().createDescriptorPool(createInfo);
}

void DescriptorSets::allocateSets()
{
    if (!_descriptorPool)
        return;
    for (const std::pair<const std::string, vk::DescriptorSetLayout> &descriptorSetLayout : _descriptorSetLayouts)
    {
        uint32_t count = _descriptorSetsCounts[descriptorSetLayout.first];
        if (!count)
            continue;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(count, descriptorSetLayout.second);

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(_descriptorPool).setDescriptorSetCount(count).setSetLayouts(descriptorSetLayouts);

        if (auto p = _device.lock<Device>())
            _descriptorSets[descriptorSetLayout.first] = p->device().allocateDescriptorSets(allocateInfo);
        _descriptorSetResource[descriptorSetLayout.first] = std::vector<std::vector<DescriptorSetsResource>>(
            count, std::vector<DescriptorSetsResource>(_descriptorSetLayoutInfos.at(descriptorSetLayout.first).size()));
    }
}

const std::string DescriptorSets::addDescriptorSetCreateConfig(std::string name,
                                                               std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> descriptorTypes, uint32_t count,
                                                               const std::pair<VkResourceManagerHolder::WeakReference, std::string> &layout)
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

    if (auto p = layout.first.lock<DescriptorSets>())
    {
        connectTo(layout.first);
        _descriptorSetLayouts[name] = p->layout(layout.second);
    }
    else
    {
        uint32_t bindingCount = 0;
        std::vector<vk::DescriptorSetLayoutBinding> bindings(descriptorTypes.size());
        for (const std::pair<vk::DescriptorType, vk::ShaderStageFlags> &descriptorType : descriptorTypes)
            bindings[bindingCount]
                .setDescriptorCount(1)
                .setDescriptorType(descriptorType.first)
                .setPImmutableSamplers(nullptr)
                .setBinding(bindingCount++)
                .setStageFlags(descriptorType.second);

        vk::DescriptorSetLayoutCreateInfo createInfo;
        createInfo.setBindings(bindings);
        vk::DescriptorSetLayout layout;
        if (auto p = _device.lock<Device>())
            layout = p->device().createDescriptorSetLayout(createInfo);
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

void DescriptorSets::writeSets(const std::string &name, uint32_t binding,
                               std::vector<std::pair<vk::DescriptorBufferInfo, VkResourceManagerHolder::WeakReference>> bufferInfos,
                               std::vector<std::pair<vk::DescriptorImageInfo, VkResourceManagerHolder::WeakReference>> imageInfos, uint32_t count)
{
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        writeDescriptorSets[i]
            .setDstBinding(binding)
            .setDstSet(_descriptorSets.at(name)[i])
            .setDescriptorType(_descriptorSetLayoutInfos.at(name)[binding].first);
        if (bufferInfos.size())
        {
            std::pair<vk::DescriptorBufferInfo, VkResourceManagerHolder::WeakReference> &info = bufferInfos.at(i);
            vk::DescriptorBufferInfo &writeInfo = info.first;
            DescriptorSetsResource &resource = _descriptorSetResource.at(name)[i][binding];
            if (resource.buffer.lock())
                removeExtraSubresource(resource.buffer);
            if (auto p = info.second.lock<Buffer>())
                writeDescriptorSets[i].setBufferInfo(writeInfo.setBuffer(p->buffer()));
            resource.buffer = info.second;
            addExtraSubresource(resource.buffer);
        }
        if (imageInfos.size())
        {
            std::pair<vk::DescriptorImageInfo, VkResourceManagerHolder::WeakReference> &info = imageInfos.at(i);
            vk::DescriptorImageInfo &writeInfo = info.first;
            DescriptorSetsResource &resource = _descriptorSetResource.at(name)[i][binding];
            if (resource.image.lock())
                removeExtraSubresource(resource.image);
            if (auto p = info.second.lock<Image>())
                writeDescriptorSets[i].setImageInfo(writeInfo.setImageView(p->view()));
            resource.image = info.second;
            addExtraSubresource(resource.image);
        }
    }
    if (auto p = _device.lock<Device>())
        p->device().updateDescriptorSets(writeDescriptorSets, nullptr);
}

void DescriptorSets::addDescriptorSetCreateConfigWithJson(const json &config)
{
    for (const json &descriptorSetCreateConfig : config)
        addDescriptorSetCreateConfig(descriptorSetCreateConfig["name"],
                                     JsonConfigReader::getDescriptorTypeShaderStageWithJson(descriptorSetCreateConfig["layoutConfig"]),
                                     getCount(descriptorSetCreateConfig));
}

uint32_t DescriptorSets::getCount(const json &config)
{
    const json &countJson = config["count"];
    if (countJson.is_string())
    {
        if (std::string(countJson) == "swapchainImageCount")
        {
            if (!config.count("swapchainName"))
                throw std::runtime_error("Config Error: There is not swapchainName in DescriptorSets Config.");

            if (auto p = resourceManager().resource(Vkbase::VkResourceType::Swapchain, config["swapchainName"]).lock<Swapchain>())
                return p->imageNames().size();
            else
                throw std::runtime_error("Config Error: There is not a swapchain named " + std::string(config["swapchainName"]));
        }
    }
    else if (countJson.is_number_integer())
        return countJson;

    throw std::runtime_error("Config Error: There is a error in DescriptorSets Config.");
}

void DescriptorSets::writeSetsWithJson(const json &config)
{
    for (const json &writeConfig : config)
    {
        const std::string &type = writeConfig["type"];

        int count = 0;
        {
            const json &countJson = writeConfig["count"];
            if (countJson.is_string())
            {
                if (std::string(countJson) == "auto")
                {
                    if (auto p = resourceManager().resource(Vkbase::VkResourceType::Swapchain, writeConfig["detail"]["swapchainName"]).lock<Swapchain>())
                    {
                        count = p->imageNames().size();
                    }
                    else
                    {
                        throw std::runtime_error("Here is not swapchain named " + std::string(writeConfig["detail"]["swapchainName"]));
                    }
                }
            }
            else if (countJson.is_number_integer())
                count = countJson;
        }

        if (type == "Image")
        {
            std::vector<std::pair<vk::DescriptorImageInfo, VkResourceManagerHolder::WeakReference>> imageInfos(count);
            for (uint32_t i = 0; i < imageInfos.size(); ++i)
            {
                const json &imageInfoJson = writeConfig["detail"]["imageInfos"][i];
                imageInfos[i].first.setImageLayout(JsonConfigReader::getImageLayoutWithJson(imageInfoJson["imageLayout"]));
                imageInfos[i].second = resourceManager().resource(Vkbase::VkResourceType::Image, imageInfoJson["imageName"]);
                if (imageInfoJson.count("samplerName") && imageInfoJson["samplerName"].is_string())
                {
                    if (auto p = resourceManager().resource(Vkbase::VkResourceType::Sampler, imageInfoJson["samplerName"]).lock<Sampler>())
                        imageInfos[i].first.setSampler(p->sampler());
                }
            }

            writeSets(writeConfig["name"], writeConfig["binding"], {}, imageInfos, imageInfos.size());
        }
        else if (type == "Buffer")
        {
            std::vector<std::pair<vk::DescriptorBufferInfo, VkResourceManagerHolder::WeakReference>> bufferInfos(count);
            for (uint32_t i = 0; i < bufferInfos.size(); ++i)
            {
                const json *pBufferInfoJson;
                if (writeConfig["count"] == "auto")
                    pBufferInfoJson = &writeConfig["detail"]["bufferInfo"];
                else
                    pBufferInfoJson = &writeConfig["detail"]["bufferInfos"][i];
                const json &bufferInfoJson = *pBufferInfoJson;

                auto buffer = resourceManager().resource(Vkbase::VkResourceType::Buffer,
                                                         writeConfig["count"] == "auto" ? std::string(bufferInfoJson["bufferName"]) + "_" + std::to_string(i)
                                                                                        : std::string(bufferInfoJson["bufferName"]));
                if (auto p = buffer.lock<Buffer>())
                    bufferInfos[i].first.setOffset(bufferInfoJson["offset"]).setRange(p->size());
                else
                    throw std::runtime_error("Config Error");
                bufferInfos[i].second = buffer;
            }
            writeSets(writeConfig["name"], writeConfig["binding"], bufferInfos, {}, bufferInfos.size());
        }
        else if (type == "Framebuffer")
        {
            std::vector<std::pair<vk::DescriptorImageInfo, VkResourceManagerHolder::WeakReference>> imageInfos(count);
            for (uint32_t i = 0; i < imageInfos.size(); ++i)
            {
                const json &imageInfoJson = writeConfig["detail"]["imageInfo"];
                imageInfos[i].first.setImageLayout(JsonConfigReader::getImageLayoutWithJson(imageInfoJson["imageLayout"]));

                imageInfos[i].second = resourceManager().resource(
                    Vkbase::VkResourceType::Image, "Framebuffer_Image_" + std::string(imageInfoJson["imageName"]) + std::string("_") + std::to_string(i));

                if (imageInfoJson.count("samplerName") && imageInfoJson["samplerName"].is_string())
                {
                    if (auto p = resourceManager().resource(Vkbase::VkResourceType::Sampler, imageInfoJson["samplerName"]).lock<Sampler>())
                        imageInfos[i].first.setSampler(p->sampler());
                    else
                        imageInfos[i].first.setSampler(Resources::ResourceManager::instance()
                                                           .getResource<Resources::SamplerResource>(_device.lock()->name(), Sampler::getDefaultCreateInfo())
                                                           .sampler());
                }
            }

            writeSets(writeConfig["name"], writeConfig["binding"], {}, imageInfos, imageInfos.size());
        }
    }
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
}; // namespace Vkbase