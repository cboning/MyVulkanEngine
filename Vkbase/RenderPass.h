#pragma once
#include "VkGpuResourceBase.h"
#include "json.hpp"

using json = nlohmann::json;

namespace Vkbase
{
class Device;
class Framebuffer;
class Pipeline;
class DescriptorSets;
class CommandBuffer;
struct VertexInfo;
struct PipelineCreateInfo;
class RenderPass : public VkGpuResourceBase
{
    friend class VkResourceManager;

public:
    uint32_t attachmentCount() const;
    const vk::RenderPass &renderPass() const;
    const std::vector<vk::Format> &attachmentFormats() const;
    const Framebuffer &createFramebuffer(const std::string &resourceName, const std::vector<std::string> &attachmentNames, uint32_t width,
                                         uint32_t height) const;
    std::vector<std::string> createFramebuffer(const std::string &resourceName, const json &config, uint32_t width, uint32_t height,
                                               const std::string &swapchainName = "", vk::Format depthFormat = vk::Format()) const;
    const Pipeline &createPipeline(const std::string &resourceName, const PipelineCreateInfo &createInfo) const;
    void createPipelines(const json &config, const std::unordered_map<std::string, VertexInfo> &vertexInfos,
                         const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> &descriptorSetLayouts,
                         const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> &viewportInfos);
    void begin(CommandBuffer *pCommandBuffer, const std::string &framebufferName, const std::vector<vk::ClearValue> &clearValues);
    void end(CommandBuffer *pCommandBuffer);
    DescriptorSets &descriptorSets();

private:
    RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo);
    RenderPass(const std::string &resourceName, const std::string &deviceName, const json &config, const std::string &swapchainName, vk::Format depthFormat);
    ~RenderPass() override;
    vk::RenderPass _renderPass;
    uint32_t _attachmentCount;
    DescriptorSets &_descriptorSets;
    std::vector<vk::Format> _attachmentFormats;
};
} // namespace Vkbase