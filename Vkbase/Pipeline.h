#pragma once
#include "ResourceBase.h"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace Vkbase
{
    class Device;
    class Pipeline : public ResourceBase
    {
    public:
        struct ShaderInfo
        {
            std::string filename;
            std::string stageName = "main";
            vk::ShaderStageFlagBits stageFlags;
            ShaderInfo(std::string filename, std::string stageName, vk::ShaderStageFlagBits stageFlags)
                : filename(filename), stageName(stageName), stageFlags(stageFlags)
            {
            }
        };

        struct VertexInfo
        {
            std::vector<vk::VertexInputAttributeDescription> inputAttributes;
            std::vector<vk::VertexInputBindingDescription> inputBindings;
            VertexInfo(const std::vector<vk::VertexInputAttributeDescription> &inputAttributes, const std::vector<vk::VertexInputBindingDescription> &inputBindings)
                : inputAttributes(inputAttributes), inputBindings(inputBindings)
            {}
        };

        static const std::vector<ShaderInfo> getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename, const std::string &vertexShaderName, const std::string &fragmentShaderName);

        Pipeline(const std::string &resourceName,
                const std::string &renderPassName,
                const std::string &deviceName,
                const std::vector<ShaderInfo> &shaderInfos,
                const VertexInfo &vertexInfo,
                std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);

        ~Pipeline();

    private:
        vk::Pipeline _pipeline;
        vk::PipelineLayout _pipelineLayout;
        vk::DescriptorSetLayout _descriptorSetLayout;
        std::vector<vk::ShaderModule> _shaderModules;
        const Device &_device;
        vk::ShaderModule createShaderModule(std::string filename);
        void createPipeline(const std::string &renderPassName,
                            const std::vector<ShaderInfo> &shaderInfos,
                            const VertexInfo &vertexInfo,
                            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
        std::vector<vk::PipelineShaderStageCreateInfo> getShaderStageInfos(const std::vector<ShaderInfo> &shaderInfos);
    };
}