#include "Pipeline.h"
#include "Device.h"
#include "RenderPass.h"
#include <fstream>

namespace Vkbase
{
Pipeline::Pipeline(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName, const PipelineCreateInfo &createInfo,
                   bool computePipeline)
    : VkGpuResourceBase(Vkbase::VkResourceType::Pipeline, resourceName, *dynamic_cast<Device *>(resourceManager().resource(VkResourceType::Device, deviceName)))
{
    createPipeline(renderPassName, createInfo, computePipeline);
}

Pipeline::~Pipeline()
{
    vk::Device device = _device.device();
    std::vector<vk::ShaderModule> shaderModules = _shaderModules;
    vk::Pipeline pipeline = _pipeline;
    vk::PipelineLayout pipelineLayout = _pipelineLayout;

    _onDelayDestroy = [device, shaderModules, pipeline, pipelineLayout]()
    {
        device.destroy(pipeline);
        device.destroy(pipelineLayout);
        for (const vk::ShaderModule &shaderModule : shaderModules)
            device.destroy(shaderModule);
    };
}

void Pipeline::createPipeline(const std::string &renderPassName, const PipelineCreateInfo &createInfo, bool computePipeline)
{
    std::vector<vk::PipelineShaderStageCreateInfo> stages = getShaderStageInfos(createInfo.shaderInfos);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setSetLayouts(createInfo.descriptorSetLayouts);

    _pipelineLayout = _device.device().createPipelineLayout(pipelineLayoutInfo);
    if (computePipeline)
    {
        _pipelineBindPoint = vk::PipelineBindPoint::eCompute;
        vk::ComputePipelineCreateInfo pipelineInfo;
        pipelineInfo.setStage(stages[0]).setLayout(_pipelineLayout);

        vk::ResultValue result = _device.device().createComputePipeline(nullptr, pipelineInfo);

        if (result.result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create compute pipeline!");

        _pipeline = result.value;
        return;
    }

    _subpass = createInfo.renderInfo.subpass;

    _pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.setVertexBindingDescriptions(createInfo.vertexInfo.inputBindings).setVertexAttributeDescriptions(createInfo.vertexInfo.inputAttributes);
    createInfo.renderInfo.multisampleStateInfo.setPSampleMask(_sampleMask);
    vk::GraphicsPipelineCreateInfo pipelineInfo = createInfo.renderInfo.getGraphicsPipelineCreateInfo();
    pipelineInfo.setStages(stages).setLayout(_pipelineLayout).setPVertexInputState(&vertexInputState);

    if (!renderPassName.empty())
    {
        const RenderPass *renderPassResource =
            dynamic_cast<const RenderPass *>(connectTo(resourceManager().resource(VkResourceType::RenderPass, renderPassName)));
        if (!renderPassResource)
            throw std::runtime_error("RenderPass resource not found: " + renderPassName);
        const vk::RenderPass &renderPass = renderPassResource->renderPass();
        pipelineInfo.setRenderPass(renderPass);
    }

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
    std::vector<char> vertexBuffer(fileSize);

    vertexFile.seekg(0);
    vertexFile.read(vertexBuffer.data(), fileSize);
    vertexFile.close();

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(fileSize).setPCode(reinterpret_cast<const uint32_t *>(vertexBuffer.data()));

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
        stageInfo.setModule(createShaderModule(shaderInfo.filename)).setStage(shaderInfo.stageFlag).setPName(shaderInfo.stageName.c_str());
        stages.push_back(stageInfo);
    }
    return stages;
}

const std::vector<ShaderInfo> Pipeline::getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename,
                                                         const std::string &vertexShaderName = "main", const std::string &fragmentShaderName = "main")
{
    return {{vertexShaderFilename, vertexShaderName, vk::ShaderStageFlagBits::eVertex},
            {fragmentShaderFilename, fragmentShaderName, vk::ShaderStageFlagBits::eFragment}};
}

PipelineRenderInfo Pipeline::getDefaultRenderInfo()
{
    PipelineRenderInfo renderInfo;

    renderInfo.inputAssemblyStateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);

    renderInfo.viewportStateInfo.setViewportCount(1).setScissorCount(1);

    renderInfo.multisampleStateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);

    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone).setFrontFace(vk::FrontFace::eClockwise).setLineWidth(1.0f);

    renderInfo.depthStencilStateInfo.setDepthTestEnable(vk::False)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f);

    renderInfo.colorBlendStateInfo.setLogicOp(vk::LogicOp::eCopy);

    renderInfo.dynamicStatus = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    renderInfo.dynamicStateInfo.setDynamicStates(renderInfo.dynamicStatus);
    return renderInfo;
}

uint32_t Pipeline::subpass() const { return _subpass; }

const vk::Pipeline &Pipeline::pipeline() const { return _pipeline; }

const vk::PipelineLayout &Pipeline::layout() const { return _pipelineLayout; }

vk::PipelineBindPoint Pipeline::pipelineBindPoint() { return _pipelineBindPoint; }
} // namespace Vkbase