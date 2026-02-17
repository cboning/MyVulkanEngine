#pragma once
#include "VkGpuResourceBase.h"
#include "json.hpp"

using json = nlohmann::json;

namespace Vkbase
{
struct VertexInfo;
struct PipelineCreateInfo;
class RenderPass : public VkGpuResourceBase
{
    friend class VkResourceManager;

public:
    uint32_t attachmentCount() const;
    const vk::RenderPass &renderPass() const;
    const std::vector<vk::Format> &attachmentFormats() const;
    VkResourceManagerHolder::WeakReference createFramebuffer(const std::string &resourceName, const std::vector<std::string> &attachmentNames, uint32_t width,
                                         uint32_t height) const;
    std::vector<std::string> createFramebuffer(const std::string &resourceName, const json &config, uint32_t width, uint32_t height,
                                               const std::string &swapchainName = "", vk::Format depthFormat = vk::Format()) const;
    VkResourceManagerHolder::WeakReference createPipeline(const std::string &resourceName, const PipelineCreateInfo &createInfo) const;
    void createPipelines(const json &config, const std::unordered_map<std::string, VertexInfo> &vertexInfos,
                         const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> &descriptorSetLayouts,
                         const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> &viewportInfos);
    void begin(const VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &framebufferName, const std::vector<vk::ClearValue> &clearValues,
               vk::SubpassContents subpassContents = vk::SubpassContents::eInline);
    static void setViewportScissor(const VkResourceManagerHolder::WeakReference &commandBuffer, const vk::Extent2D &extent);
    void end(const VkResourceManagerHolder::WeakReference &commandBuffer);
    VkResourceManagerHolder::WeakReference descriptorSets() const;

private:
    RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo);
    RenderPass(const std::string &resourceName, const std::string &deviceName, const json &config);
    ~RenderPass() override;
    vk::RenderPass _renderPass;
    uint32_t _attachmentCount;
    VkResourceManagerHolder::WeakReference _descriptorSets;
    std::vector<vk::Format> _attachmentFormats;
};
} // namespace Vkbase