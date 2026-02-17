#pragma once
#include <assimp/material.h>
#include <json.hpp>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

using json = nlohmann::json;

class JsonConfigReader
{
  public:
    static aiTextureType getTextureTypeWithString(const std::string &textureType);
    static vk::Format getFormatWithJson(const std::string &format, const std::string &swapchainName = "", vk::Format depthFormat = vk::Format());
    static std::string getJsonWithFormat(const vk::Format &format);
    static vk::ImageLayout getImageLayoutWithJson(const std::string &imageLayout);
    static vk::PipelineStageFlagBits getPipelineStageFlagBitsWithJson(const std::string &pipelineStage);
    static vk::SampleCountFlagBits getSampleCountFlagBitsWithJson(uint32_t sampleCount);
    static vk::AttachmentLoadOp getAttachmentLoadOpWithJson(const std::string &loadOp);
    static vk::AttachmentStoreOp getAttachmentStoreOpWithJson(const std::string &storeOp);
    static vk::AccessFlagBits getAccessFlagBitsWithJson(const std::string &access);
    static vk::PipelineStageFlags getPipelineStageFlagsWithJson(const std::vector<std::string> &pipelineStageFlags);
    static vk::AccessFlags getAccessFlagsWithJson(const std::vector<std::string> &accessFlags);
    static std::vector<vk::AttachmentDescription> getAttachmentsWithJson(const json &config, vk::Format depthFormat);
    static std::vector<std::vector<vk::AttachmentReference>> getAttachmentRefsWithJson(const json &config);
    static std::vector<vk::SubpassDescription> getSubpassesWithJson(const json &config,
                                                                    const std::vector<std::vector<vk::AttachmentReference>> &attachmentRefs);
    static std::vector<vk::SubpassDependency> getSubpassDependenciesWithJson(const json &config);
    static vk::RenderPassCreateInfo getRenderPassCreateInfo(const std::vector<vk::AttachmentDescription> &attachments,
                                                            const std::vector<vk::SubpassDescription> &subpasses,
                                                            const std::vector<vk::SubpassDependency> &subpassDependencies);
    static vk::ImageType getImageTypeWithJson(const std::string &imageType);
    static vk::ImageViewType getImageViewTypeWithJson(const std::string &imageViewType);
    static vk::ImageUsageFlagBits getImageUsageFlagBitsWithJson(const std::string &imageUsageFlagBits);
    static vk::ImageUsageFlags getImageUsageFlagsWithJson(const json &imageUsageFlags);
    static vk::PipelineInputAssemblyStateCreateInfo getPipelineInputAssemblyStateCreateInfoWithJson(const json &config);
    static vk::Bool32 getBooleanWithJson(const json &config);
    static vk::PrimitiveTopology getPrimitiveTopologyWithJson(const json &primitiveTopology);
    static std::vector<vk::Rect2D> getScissorsWithJson(const json &config);
    static vk::Extent2D getExtent2DWithJson(const json &config);
    static vk::Offset2D getOffset2DWithJson(const json &config);
    static vk::Rect2D getScissorWithJson(const json &config);
    static std::vector<vk::Viewport> getViewportsWithJson(const json &config);
    static vk::Viewport getViewportWithJson(const json &config);
    static vk::PipelineRasterizationStateCreateInfo getPipelineRasterizationStateCreateInfoWithJson(const json &config);
    static vk::FrontFace getFrontFaceWithJson(const json &config);
    static vk::PolygonMode getPolygonModeWithJson(const json &config);
    static vk::PipelineMultisampleStateCreateInfo getPipelineMultisampleStateCreateInfo(const json &config);
    static bool textInList(const std::string &text, const std::vector<std::string> &list);
    static vk::CullModeFlags getCullModeFlagsWithJson(const json &config);
    static vk::CullModeFlagBits getCullModeFlagBitsWithJson(const json &config);
    static vk::PipelineTessellationStateCreateInfo getPipelineTessellationStateCreateInfoWithJson(const json &config);
    static vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfoWithJson(const json &config);
    static vk::StencilOpState getStencilOpStateWithJson(const json &config);
    static vk::CompareOp getCompareOpWithJson(const json &config);
    static vk::StencilOp getStencilOpWithJson(const json &config);
    static vk::PipelineDepthStencilStateCreateFlags getPipelineDepthStencilStateCreateInfoFlagsWithJson(const json &config);
    static vk::PipelineDepthStencilStateCreateFlagBits getPipelineDepthStencilStateCreateInfoFlagBitsWithJson(const json &config);
    static vk::PipelineColorBlendStateCreateInfo getPipelineColorBlendStateCreateInfoWithJson(const json &config);
    static vk::PipelineColorBlendStateCreateFlags getPipelineColorBlendStateCreateFlagsWithJson(const json &config);
    static vk::PipelineColorBlendStateCreateFlagBits getPipelineColorBlendStateCreateFlagBitsWithJson(const json &config);
    static vk::LogicOp getLogicOp(const json &config);
    static std::vector<vk::PipelineColorBlendAttachmentState> getPipelineColorBlendAttachmentStatesWithJson(const json &config);
    static vk::PipelineColorBlendAttachmentState getPipelineColorBlendAttachmentStateWithJson(const json &config);
    static vk::BlendOp getBlendOpWithJson(const json &config);
    static vk::ColorComponentFlags getColorComponentFlagsWithJson(const json &config);
    static vk::ColorComponentFlagBits getColorComponentFlagBitsWithJson(const json &config);
    static vk::BlendFactor getBlendFactorWithJson(const json &config);
    static std::vector<vk::DynamicState> getDynamicStatusWithJson(const json &config);
    static vk::DynamicState getDynamicStateWithJson(const json &config);
    static std::vector<std::pair<vk::DescriptorType,vk::ShaderStageFlags>> getDescriptorTypeShaderStageWithJson(const json & config);
    static vk::ShaderStageFlags getShaderStageFlagsWithJson(const json &config);
    static vk::DescriptorType getDescriptorTypeWithJson(const json &config);
    static vk::ShaderStageFlagBits getShaderStageFlagBitsWithJson(
        const json &config);

    static json load(const std::string &filename);
};