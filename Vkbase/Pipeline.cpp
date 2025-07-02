#include "Pipeline.h"
#include "Device.h"
#include "RenderPass.h"
#include <fstream>

namespace Vkbase
{
    Pipeline::Pipeline(const std::string &resourceName,
                       const std::string &deviceName,
                       const std::string &renderPassName,
                       const PipelineCreateInfo &createInfo)
        : ResourceBase(Vkbase::ResourceType::Pipeline, resourceName), _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(ResourceType::Device, deviceName))))
    {
        createPipeline(renderPassName, createInfo);
    }

    Pipeline::~Pipeline()
    {
        vk::Device device = _device.device();
        device.destroy(_pipeline);
        device.destroy(_pipelineLayout);
        for (const vk::ShaderModule &shaderModule : _shaderModules)
            device.destroy(shaderModule);
    }

    void Pipeline::createPipeline(const std::string &renderPassName,
                                  const PipelineCreateInfo &createInfo)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> stages = getShaderStageInfos(createInfo.shaderInfos);

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayouts(createInfo.descriptorSetLayouts);
        _pipelineLayout = _device.device().createPipelineLayout(pipelineLayoutInfo);
        const RenderPass *renderPassResource = dynamic_cast<const RenderPass *>(
            connectTo(resourceManager().resource(ResourceType::RenderPass, renderPassName)));
        if (!renderPassResource)
            throw std::runtime_error("RenderPass resource not found: " + renderPassName);
        const vk::RenderPass &renderPass = renderPassResource->renderPass();

        vk::PipelineVertexInputStateCreateInfo vertexInputState;
        vertexInputState.setVertexBindingDescriptions(createInfo.vertexInfo.inputBindings)
            .setVertexAttributeDescriptions(createInfo.vertexInfo.inputAttributes);

        vk::GraphicsPipelineCreateInfo pipelineInfo = createInfo.renderInfo.getGraphicsPipelineCreateInfo();
        pipelineInfo.setStages(stages)
            .setLayout(_pipelineLayout)
            .setRenderPass(renderPass)
            .setPVertexInputState(&vertexInputState);

        vk::ResultValue result = _device.device().createGraphicsPipeline(nullptr, pipelineInfo);

        if (result.result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create graphics pipeline!");

        _pipeline = result.value;
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

    const std::vector<ShaderInfo> Pipeline::getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename, const std::string &vertexShaderName = "main", const std::string &fragmentShaderName = "main")
    {
        return {{vertexShaderFilename, vertexShaderName, vk::ShaderStageFlagBits::eVertex}, {fragmentShaderFilename, fragmentShaderName, vk::ShaderStageFlagBits::eFragment}};
    }

    PipelineRenderInfo Pipeline::getDefaultRenderInfo()
    {
        PipelineRenderInfo renderInfo;

        renderInfo.inputAssemblyStateInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(vk::False);

        renderInfo.viewportStateInfo.setViewportCount(1)
            .setScissorCount(1);

        renderInfo.multisampleStateInfo.setSampleShadingEnable(vk::False)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);

        renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eFront)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setDepthClampEnable(vk::False)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f)
            .setDepthBiasEnable(vk::False)
            .setDepthBiasClamp(0.0f)
            .setDepthBiasConstantFactor(0.0f)
            .setDepthBiasSlopeFactor(0.0f);

        renderInfo.depthStencilStateInfo.setDepthTestEnable(vk::False)
            .setDepthWriteEnable(vk::True)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthBoundsTestEnable(vk::False)
            .setMinDepthBounds(0.0f)
            .setMaxDepthBounds(1.0f)
            .setStencilTestEnable(vk::False);

        renderInfo.colorBlendStateInfo.setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

        renderInfo.dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        renderInfo.dynamicStateInfo.setDynamicStates(renderInfo.dynamicStates);
        return renderInfo;
    }
}