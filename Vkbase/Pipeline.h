#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
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
        std::vector<vk::DynamicState> dynamicStates;

        vk::GraphicsPipelineCreateInfo getGraphicsPipelineCreateInfo()
        {
            colorBlendStateInfo.setAttachments(blendAttachments);
            dynamicStateInfo.setDynamicStates(dynamicStates);
            vk::GraphicsPipelineCreateInfo createInfo;
            return createInfo.setPInputAssemblyState(&inputAssemblyStateInfo)
                .setPTessellationState(&tessellationStateInfo)
                .setPViewportState(&viewportStateInfo)
                .setPRasterizationState(&rasterizationStateInfo)
                .setPMultisampleState(&multisampleStateInfo)
                .setPDepthStencilState(&depthStencilStateInfo)
                .setPColorBlendState(&colorBlendStateInfo)
                .setPDynamicState(&dynamicStateInfo)
                .setSubpass(subpass)
                .setBasePipelineHandle(basePipelineHandle)
                .setBasePipelineIndex(basePipelineIndex);
        }
        PipelineRenderInfo(
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList),
            vk::PipelineTessellationStateCreateInfo tessellationState = {},
            vk::PipelineViewportStateCreateInfo viewportState = {},
            vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eFront).setFrontFace(vk::FrontFace::eClockwise).setDepthClampEnable(vk::False).setRasterizerDiscardEnable(vk::False).setDepthBiasEnable(vk::False).setDepthBiasClamp(0.0f).setDepthBiasConstantFactor(0.0f).setDepthBiasSlopeFactor(0.0f),
            vk::PipelineMultisampleStateCreateInfo multisampleState = {},
            vk::PipelineDepthStencilStateCreateInfo depthStencilState = vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(vk::True).setDepthWriteEnable(vk::True).setDepthCompareOp(vk::CompareOp::eLess).setMinDepthBounds(0.0f).setMaxDepthBounds(1.0f).setStencilTestEnable(vk::False),
            vk::PipelineColorBlendStateCreateInfo colorBlendState = {},
            vk::PipelineDynamicStateCreateInfo dynamicState = {},
            vk::RenderPass renderPass = {},
            uint32_t subpass = {},
            vk::Pipeline basePipelineHandle = {},
            int32_t basePipelineIndex = {})
            : inputAssemblyStateInfo{inputAssemblyState}, tessellationStateInfo{tessellationState}, viewportStateInfo{viewportState}, rasterizationStateInfo{rasterizationState}, multisampleStateInfo{multisampleState}, depthStencilStateInfo{depthStencilState}, colorBlendStateInfo{colorBlendState}, dynamicStateInfo{dynamicState}, subpass{subpass}, basePipelineHandle{basePipelineHandle}, basePipelineIndex{basePipelineIndex}
        {
        }
    };
    struct PipelineCreateInfo
    {
        const std::vector<ShaderInfo> &shaderInfos;
        const VertexInfo &vertexInfo;
        const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts;
        PipelineRenderInfo &renderInfo;
        PipelineCreateInfo(const std::vector<ShaderInfo> &shaderInfos,
                            const VertexInfo &vertexInfo,
                            const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts,
                            PipelineRenderInfo &renderInfo)
            : shaderInfos(shaderInfos), vertexInfo(vertexInfo), descriptorSetLayouts(descriptorSetLayouts), renderInfo(renderInfo)
        {}
    };
    class Device;
    class Pipeline : public ResourceBase
    {
    public:

        static const std::vector<ShaderInfo> getDefaultShader(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename, const std::string &vertexShaderName, const std::string &fragmentShaderName);
        static PipelineRenderInfo getDefaultRenderInfo();
        const vk::Pipeline &pipeline() const;
        Pipeline(const std::string &resourceName,
                 const std::string &deviceName,
                 const std::string &renderPassName,
                 const PipelineCreateInfo &createInfo);

        ~Pipeline();

    private:
        vk::Pipeline _pipeline;
        vk::PipelineLayout _pipelineLayout;
        vk::DescriptorSetLayout _descriptorSetLayout;
        std::vector<vk::ShaderModule> _shaderModules;
        const Device &_device;
        vk::ShaderModule createShaderModule(std::string filename);
        void createPipeline(const std::string &renderPassName,
                            const PipelineCreateInfo &createInfo);
        std::vector<vk::PipelineShaderStageCreateInfo> getShaderStageInfos(const std::vector<ShaderInfo> &shaderInfos);
    };

}