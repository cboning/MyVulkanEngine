#include "Pipeline.h"
#include "Device.h"
#include <fstream>

namespace Vkbase
{
    Pipeline::Pipeline(const std::string &resourceName,
                const std::string &renderPassName,
                const std::string &deviceName,
                const std::vector<ShaderInfo> &shaderInfos,
                const VertexInfo &vertexInfo,
                std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
        : ResourceBase(Vkbase::ResourceType::Pipeline, resourceName), _device(*dynamic_cast<const Device *>(resourceManager().resource(ResourceType::Device, deviceName)))
    {
        createPipeline(renderPassName, shaderInfos, vertexInfo, descriptorSetLayouts);
    }

    Pipeline::~Pipeline()
    {
        for (const vk::ShaderModule &shaderModule : _shaderModules)
            _device.device().destroy(shaderModule);
    }

    void Pipeline::createPipeline(const std::string &renderPassName,
                                const std::vector<ShaderInfo> &shaderInfos,
                                const VertexInfo &vertexInfo,
                                std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
    {
        vk::GraphicsPipelineCreateInfo createInfo;
        std::vector<vk::PipelineShaderStageCreateInfo> stages = getShaderStageInfos(shaderInfos);
        
        createInfo.setStages(stages);

        
    }

    vk::ShaderModule Pipeline::createShaderModule(std::string filename)
    {
        std::ifstream vertexFile(filename, std::ios::ate | std::ios::binary);
        if (!vertexFile.is_open())
            throw std::runtime_error("Failed to open shader file!");
        size_t fileSize = (size_t)vertexFile.tellg();
        char vertexBuffer[fileSize];

        vertexFile.seekg(0);
        vertexFile.read(vertexBuffer, fileSize);
        vertexFile.close();

        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setCodeSize(fileSize)
            .setPCode((uint32_t *)vertexBuffer);

        vk::ShaderModule shaderModule = _device.device().createShaderModule(createInfo);
        _shaderModules.push_back(shaderModule);
        return shaderModule;
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Pipeline::getShaderStageInfos(const std::vector<ShaderInfo> &shaderInfos)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> stages;
        stages.reserve(shaderInfos.size());
        for (const ShaderInfo &shaderInfo : shaderInfos)
        {
            vk::PipelineShaderStageCreateInfo stageInfo;
            stageInfo.setModule(createShaderModule(shaderInfo.filename))
                .setStage(shaderInfo.stageFlags)
                .setPName(shaderInfo.stageName.c_str());
            stages.push_back(stageInfo);
        }
        return stages;
    }

    const std::vector<Pipeline::ShaderInfo> getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename, const std::string &vertexShaderName = "main", const std::string &fragmentShaderName = "main")
    {
        return {{vertexShaderFilename, vertexShaderName, vk::ShaderStageFlagBits::eVertex}, {fragmentShaderFilename, fragmentShaderName, vk::ShaderStageFlagBits::eFragment}};
    }
}