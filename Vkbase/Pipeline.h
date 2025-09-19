#pragma once
#include "../JsonConfigReader/JsonConfigReader.h"
#include "ResourceBase.h"
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
    ShaderInfo() {}

    std::vector<ShaderInfo> getShaderInfoWithJson(const json &config)
    {
        std::vector<ShaderInfo> shaderInfos;
        shaderInfos.reserve(config.size());
        for (const json &shaderInfoJson : config)
            shaderInfos.emplace_back(shaderInfoJson["filename"], shaderInfoJson["stageName"],
                                     JsonConfigReader::getPipelineStageFlagBitsWithJson(shaderInfoJson["stage"]));
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
        : inputAssemblyStateInfo{inputAssemblyState}, tessellationStateInfo{tessellationState}, scissors(scissors), viewports(viewports),
          rasterizationStateInfo{rasterizationState}, multisampleStateInfo{multisampleState}, depthStencilStateInfo{depthStencilState},
          colorBlendStateInfo{colorBlendState}, dynamicStateInfo{dynamicState}, subpass{subpass}, basePipelineHandle{basePipelineHandle},
          basePipelineIndex{basePipelineIndex}
    {
    }
    PipelineRenderInfo(const json &config)
    {
        static const std::unordered_map<std::string, std::function<void(const json &)>> attributeLoadMap = {
            {"inputAssemblyState", [this](const json &config)
             { inputAssemblyStateInfo = JsonConfigReader::getPipelineInputAssemblyStateCreateInfoWithJson(config["inputAssemblyState"]); }},
            {"tessellationState", [this](const json &config)
             { tessellationStateInfo = JsonConfigReader::getPipelineTessellationStateCreateInfoWithJson(config["tessellationState"]); }},
            {"rasterizationState", [this](const json &config)
             { rasterizationStateInfo = JsonConfigReader::getPipelineRasterizationStateCreateInfoWithJson(config["rasterizationState"]); }},
            {"multisampleState",
             [this](const json &config) { multisampleStateInfo = JsonConfigReader::getPipelineMultisampleStateCreateInfo(config["multisampleState"]); }},
            {"depthStencilState", [this](const json &config)
             { depthStencilStateInfo = JsonConfigReader::getPipelineDepthStencilStateCreateInfoWithJson(config["depthStencilState"]); }},
            {"colorBlendState",
             [this](const json &config) {const json &colorBlendState = config["colorBlendState"]; colorBlendStateInfo = JsonConfigReader::getPipelineColorBlendStateCreateInfoWithJson(colorBlendState); if (colorBlendState.count("blendAttachments")) blendAttachments = JsonConfigReader::getPipelineColorBlendAttachmentStatesWithJson(colorBlendState["blendAttachments"]);}},
            {"dynamicStatus", [this](const json &config) { dynamicStatus = JsonConfigReader::getDynamicStatusWithJson(config["dynamicStatus"]); }},
            {"subpass", [this](const json &config) { subpass = config["subpass"]; }},
            {"basePipelineIndex", [this](const json &config) { basePipelineIndex = config["basePipelineIndex"]; }},
            {"viewportState", [this](const json &config) { const json &dynamicStatusJson = config["viewportState"]; if (dynamicStatusJson.count("scissors")) scissors = JsonConfigReader::getScissorsWithJson(dynamicStatusJson["scissors"]); if (dynamicStatusJson.count("viewports")) viewports = JsonConfigReader::getViewportsWithJson(dynamicStatusJson["viewports"]);}},
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
    PipelineRenderInfo *pRenderInfo;
    PipelineCreateInfo(const std::vector<ShaderInfo> &shaderInfos = {}, const VertexInfo &vertexInfo = {},
                       const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts = {}, PipelineRenderInfo *renderInfo = nullptr)
        : shaderInfos(shaderInfos), vertexInfo(vertexInfo), descriptorSetLayouts(descriptorSetLayouts), pRenderInfo(renderInfo)
    {
    }
};
class Device;
class Pipeline : public ResourceBase
{
    friend class ResourceManager;

  public:
    static const std::vector<ShaderInfo> getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename,
                                                          const std::string &vertexShaderName, const std::string &fragmentShaderName);
    static PipelineRenderInfo getDefaultRenderInfo();

    const vk::Pipeline &pipeline() const;
    const vk::PipelineLayout &layout() const;

  private:
    vk::Pipeline _pipeline;
    vk::PipelineLayout _pipelineLayout;
    vk::DescriptorSetLayout _descriptorSetLayout;
    std::vector<vk::ShaderModule> _shaderModules;
    vk::SampleMask _sampleMask;
    const Device &_device;
    Pipeline(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName, const PipelineCreateInfo &createInfo,
             bool computePipeline = false);
    ~Pipeline();
    vk::ShaderModule createShaderModule(std::string filename);
    void createPipeline(const std::string &renderPassName, const PipelineCreateInfo &createInfo, bool computePipelin);
    std::vector<vk::PipelineShaderStageCreateInfo> getShaderStageInfos(const std::vector<ShaderInfo> &shaderInfos);
};

} // namespace Vkbase