#include "Pipeline.h"

namespace Vkbase
{
    Pipeline::Pipeline(const std::string &resourceName, const std::string &renderPassName, const std::string &vertexFilename, const std::string &fragmentFilename, std::vector<vk::VertexInputAttributeDescription> inputAttributes, std::vector<vk::VertexInputBindingDescription> inputBindings, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, const std::string &vertexStageName = "main", const std::string &fragmentStageName = "main")
        : ResourceBase(Vkbase::ResourceType::Pipeline, resourceName)
    {
        createPipeline(renderPassName, vertexFilename, fragmentFilename, inputAttributes, inputBindings, descriptorSetLayouts, vertexStageName, fragmentStageName);
    }
    void Pipeline::createPipeline(const std::string &renderPassName, const std::string &vertexFilename, const std::string &fragmentFilename, std::vector<vk::VertexInputAttributeDescription> inputAttributes, std::vector<vk::VertexInputBindingDescription> inputBindings, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, const std::string &vertexStageName, const std::string &fragmentStageName)
    {
        vk::GraphicsPipelineCreateInfo createInfo;
        std::vector<vk::PipelineShaderStageCreateInfo> stages = getShaderStageInfos(vertexStageName, fragmentStageName);
        createInfo.setStages(stages);
        
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Pipeline::getShaderStageInfos(const std::string &vertexStageName, const std::string &fragmentStageName)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> stages(2);
        vk::PipelineShaderStageCreateInfo stage = stages.emplace_back();
        stages[0].setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(_vertexShaderModule)
            .setPName(vertexStageName.c_str());

        stages[1].setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(_fragmentShaderModule)
            .setPName(fragmentStageName.c_str());

        return stages;
    }
}