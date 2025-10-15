#pragma once
#include "../JsonConfigReader/JsonConfigReader.h"
#include "GpuResourceBase.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

namespace Vkbase
{
struct ShaderInfo
{
    std::string filename;
    std::string stageName = "main";
    vk::ShaderStageFlagBits stageFlag;
    ShaderInfo(std::string filename, std::string stageName, vk::ShaderStageFlagBits stageFlag) : filename(filename), stageName(stageName), stageFlag(stageFlag)
    {
    }

    static std::vector<ShaderInfo> getShaderInfosWithJson(const json &config)
    {
        std::vector<ShaderInfo> shaderInfos;
        shaderInfos.reserve(config.size());
        for (const json &shaderInfoJson : config)
            shaderInfos.emplace_back(shaderInfoJson["filename"], shaderInfoJson["stageName"],
                                     JsonConfigReader::getShaderStageFlagBitsWithJson(shaderInfoJson["shaderStageFlags"]));
        return shaderInfos;
    }
};

struct VertexInfo
{
    std::vector<vk::VertexInputAttributeDescription> inputAttributes;
    std::vector<vk::VertexInputBindingDescription> inputBindings;
    VertexInfo(const std::vector<vk::VertexInputAttributeDescription> &inputAttributes = {},
               const std::vector<vk::VertexInputBindingDescription> &inputBindings = {})
        : inputAttributes(inputAttributes), inputBindings(inputBindings)
    {
    }
};

struct PipelineRenderInfo
{
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
    vk::PipelineTessellationStateCreateInfo tessellationStateInfo;
    vk::PipelineViewportStateCreateInfo viewportStateInfo;
    vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo;
    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
    uint32_t subpass;
    vk::Pipeline basePipelineHandle;
    int32_t basePipelineIndex;
    std::vector<vk::PipelineColorBlendAttachmentState> blendAttachments;
    std::vector<vk::DynamicState> dynamicStatus;
    std::vector<vk::Rect2D> scissors;
    std::vector<vk::Viewport> viewports;

    vk::GraphicsPipelineCreateInfo getGraphicsPipelineCreateInfo()
    {
        colorBlendStateInfo.setAttachments(blendAttachments);
        dynamicStateInfo.setDynamicStates(dynamicStatus);
        vk::GraphicsPipelineCreateInfo createInfo;
        return createInfo.setPInputAssemblyState(&inputAssemblyStateInfo)
            .setPTessellationState(&tessellationStateInfo)
            .setPViewportState(&viewportStateInfo.setViewports(viewports).setScissors(scissors))
            .setPRasterizationState(&rasterizationStateInfo)
            .setPMultisampleState(&multisampleStateInfo)
            .setPDepthStencilState(&depthStencilStateInfo)
            .setPColorBlendState(&colorBlendStateInfo)
            .setPDynamicState(&dynamicStateInfo)
            .setSubpass(subpass)
            .setBasePipelineHandle(basePipelineHandle)
            .setBasePipelineIndex(basePipelineIndex);
    }
    PipelineRenderInfo(vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState =
                           vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList),
                       vk::PipelineTessellationStateCreateInfo tessellationState = {}, std::vector<vk::Rect2D> scissors = {},
                       std::vector<vk::Viewport> viewports = {},
                       vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
                                                                                         .setCullMode(vk::CullModeFlagBits::eFront)
                                                                                         .setFrontFace(vk::FrontFace::eClockwise)
                                                                                         .setDepthClampEnable(vk::False)
                                                                                         .setRasterizerDiscardEnable(vk::False)
                                                                                         .setDepthBiasEnable(vk::False)
                                                                                         .setDepthBiasClamp(0.0f)
                                                                                         .setDepthBiasConstantFactor(0.0f)
                                                                                         .setDepthBiasSlopeFactor(0.0f),
                       vk::PipelineMultisampleStateCreateInfo multisampleState = {},
                       vk::PipelineDepthStencilStateCreateInfo depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
                                                                                       .setDepthTestEnable(vk::True)
                                                                                       .setDepthWriteEnable(vk::True)
                                                                                       .setDepthCompareOp(vk::CompareOp::eLess)
                                                                                       .setMinDepthBounds(0.0f)
                                                                                       .setMaxDepthBounds(1.0f)
                                                                                       .setStencilTestEnable(vk::False),
                       vk::PipelineColorBlendStateCreateInfo colorBlendState = {}, vk::PipelineDynamicStateCreateInfo dynamicState = {}, uint32_t subpass = {},
                       vk::Pipeline basePipelineHandle = {}, int32_t basePipelineIndex = {})
        : inputAssemblyStateInfo{inputAssemblyState}, tessellationStateInfo{tessellationState}, rasterizationStateInfo{rasterizationState},
          multisampleStateInfo{multisampleState}, depthStencilStateInfo{depthStencilState}, colorBlendStateInfo{colorBlendState},
          dynamicStateInfo{dynamicState}, subpass{subpass}, basePipelineHandle{basePipelineHandle}, basePipelineIndex{basePipelineIndex}, scissors(scissors),
          viewports(viewports)
    {
    }
    PipelineRenderInfo(const json &config, const std::vector<vk::Rect2D> &replenishScissors = {}, const std::vector<vk::Viewport> &replenishViewports = {})
        : scissors(replenishScissors), viewports(replenishViewports)
    {
        const std::unordered_map<std::string, std::function<void(const json &)>> attributeLoadMap = {
            {"inputAssemblyState",
             [this](const json &config) { inputAssemblyStateInfo = JsonConfigReader::getPipelineInputAssemblyStateCreateInfoWithJson(config); }},
            {"tessellationState",
             [this](const json &config) { tessellationStateInfo = JsonConfigReader::getPipelineTessellationStateCreateInfoWithJson(config); }},
            {"rasterizationState",
             [this](const json &config) { rasterizationStateInfo = JsonConfigReader::getPipelineRasterizationStateCreateInfoWithJson(config); }},
            {"multisampleState", [this](const json &config) { multisampleStateInfo = JsonConfigReader::getPipelineMultisampleStateCreateInfo(config); }},
            {"depthStencilState",
             [this](const json &config) { depthStencilStateInfo = JsonConfigReader::getPipelineDepthStencilStateCreateInfoWithJson(config); }},
            {"colorBlendState",
             [this](const json &config)
             {
                 colorBlendStateInfo = JsonConfigReader::getPipelineColorBlendStateCreateInfoWithJson(config);
                 if (config.count("blendAttachments"))
                 {
                     auto tempAttachments = JsonConfigReader::getPipelineColorBlendAttachmentStatesWithJson(config["blendAttachments"]);
                     blendAttachments = tempAttachments;
                 }
             }},
            {"dynamicStatus", [this](const json &config) { dynamicStatus = JsonConfigReader::getDynamicStatusWithJson(config); }},
            {"subpass", [this](const json &config) { subpass = config; }},
            {"basePipelineIndex", [this](const json &config) { basePipelineIndex = config; }},
            {"viewportState",
             [this](const json &config)
             {
                 if (config.count("scissors"))
                     scissors = JsonConfigReader::getScissorsWithJson(config["scissors"]);
                 if (config.count("viewports"))
                     viewports = JsonConfigReader::getViewportsWithJson(config["viewports"]);
             }},
        };

        for (auto it = config.begin(); it != config.end(); ++it)
        {
            if (!attributeLoadMap.count(it.key()))
            {
                std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
                continue;
            }

            attributeLoadMap.at(it.key())(it.value());
        }
    }
};
struct PipelineCreateInfo
{
    const std::vector<ShaderInfo> &shaderInfos;
    const VertexInfo &vertexInfo;
    const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts;
    PipelineRenderInfo &renderInfo;
    PipelineCreateInfo(const std::vector<ShaderInfo> &shaderInfos, const VertexInfo &vertexInfo,
                       const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts, PipelineRenderInfo &renderInfo)
        : shaderInfos(shaderInfos), vertexInfo(vertexInfo), descriptorSetLayouts(descriptorSetLayouts), renderInfo(renderInfo)
    {
    }
};
class Device;
class Pipeline : public GpuResourceBase
{
    friend class ResourceManager;
    friend class CommandBuffer;

public:
    static const std::vector<ShaderInfo> getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename,
                                                          const std::string &vertexShaderName, const std::string &fragmentShaderName);
    static PipelineRenderInfo getDefaultRenderInfo();

private:
    vk::Pipeline _pipeline;
    vk::PipelineLayout _pipelineLayout;
    vk::DescriptorSetLayout _descriptorSetLayout;
    std::vector<vk::ShaderModule> _shaderModules;
    vk::PipelineBindPoint _pipelineBindPoint;
    vk::SampleMask _sampleMask[2] = {0xFFFFFFFF, 0xFFFFFFFF};
    Pipeline(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName, const PipelineCreateInfo &createInfo,
             bool computePipeline = false);
    ~Pipeline();
    vk::ShaderModule createShaderModule(std::string filename);
    void createPipeline(const std::string &renderPassName, const PipelineCreateInfo &createInfo, bool computePipeline);
    std::vector<vk::PipelineShaderStageCreateInfo> getShaderStageInfos(const std::vector<ShaderInfo> &shaderInfos);
    const vk::Pipeline &pipeline() const;
    const vk::PipelineLayout &layout() const;
    vk::PipelineBindPoint pipelineBindPoint();
};

} // namespace Vkbase