#pragma once
#include "ResourceBase.h"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace Vkbase
{
    class Pipeline : public ResourceBase
    {
    private:
        vk::Pipeline _pipeline;
        vk::PipelineLayout _pipelineLayout;
        vk::DescriptorSetLayout _descriptorSetLayout;
        vk::ShaderModule _vertexShaderModule, _fragmentShaderModule;
        vk::ShaderModule createShaderModule(std::string filename);
        void createPipeline(const std::string &renderPassName, const std::string &vertexFilename, const std::string &fragmentFilename, std::vector<vk::VertexInputAttributeDescription> inputAttributes, std::vector<vk::VertexInputBindingDescription> inputBindings, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, const std::string &vertexStageName, const std::string &fragmentStageName);
        std::vector<vk::PipelineShaderStageCreateInfo> getShaderStageInfos(const std::string &vertexStageName, const std::string &fragmentStageName);

    public:
        Pipeline(const std::string &resourceName, const std::string &renderPassName, const std::string &vertexFilename, const std::string &fragmentFilename, std::vector<vk::VertexInputAttributeDescription> inputAttributes, std::vector<vk::VertexInputBindingDescription> inputBindings, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, const std::string &vertexStageName, const std::string &fragmentStageName);
        
    };
}