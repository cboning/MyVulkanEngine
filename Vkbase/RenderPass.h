#pragma once
#include "ResourceBase.h"
#include "json.hpp"

using json = nlohmann::json;

namespace Vkbase
{
class Device;
class Framebuffer;
class Pipeline;
struct PipelineCreateInfo;
class RenderPass : public ResourceBase
{
    friend class ResourceManager;

  public:
    uint32_t attachmentCount() const;
    const vk::RenderPass &renderPass() const;
    const std::vector<vk::Format> &attachmentFormats() const;
    const Framebuffer &createFramebuffer(const std::string &resourceName, const std::vector<std::string> &attachmentNames, uint32_t width,
                                         uint32_t height) const;
    const Pipeline &createPipeline(const std::string &resourceName, const PipelineCreateInfo &createInfo) const;
    void begin(const vk::CommandBuffer &commandBuffer, const Framebuffer &framebuffer, std::vector<vk::ClearValue> &clearValues, vk::Extent2D &extent) const;
    void end(const vk::CommandBuffer &commandBuffer) const;

  private:
    RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo);
    RenderPass(const std::string &resourceName, const std::string &deviceName, const json &config, const std::string &swapchainName, vk::Format depthFormat);
    ~RenderPass() override;
    vk::RenderPass _renderPass;
    const Device &_device;
    uint32_t _attachmentCount;
    std::vector<vk::Format> _attachmentFormats;
    static vk::Format getFormatWithJson(const std::string &format, const std::string &swapchainName, vk::Format depthFormat);
    static vk::SampleCountFlagBits getSampleCountFlagBitWithJson(uint32_t sampleCount);
    static vk::AttachmentLoadOp getAttachmentLoadOpWithJson(const std::string &loadOp);
    static vk::AttachmentStoreOp getAttachmentStoreOpWithJson(const std::string &storeOp);
    static vk::ImageLayout getImageLayoutWithJson(const std::string &imageLayout);
    static vk::PipelineStageFlagBits getPipelineStageFlagBitWithJson(const std::string &pipelineStage);
    static vk::AccessFlagBits getAccessFlagBitWithJson(const std::string &access);
    static vk::PipelineStageFlags getPipelineStageFlagsWithJson(const std::vector<std::string> &pipelineStageFlags);
    static vk::AccessFlags getAccessFlagsWithJson(const std::vector<std::string> &accessFlags);
    static std::vector<vk::AttachmentDescription> getAttachmentsWithJson(const json &config, const std::string &swapchainName, vk::Format depthFormat);
    static std::vector<std::vector<vk::AttachmentReference>> getAttachmentRefsWithJson(const json &config);
    static std::vector<vk::SubpassDescription> getSubpassesWithJson(const json &config, const std::vector<std::vector<vk::AttachmentReference>> &attachmentRefs);
    static std::vector<vk::SubpassDependency> getSubpassDependenciesWithJson(const json &config);
    static vk::RenderPassCreateInfo getRenderPassCreateInfo(const std::vector<vk::AttachmentDescription> &attachments, const std::vector<vk::SubpassDescription> &subpasses, const std::vector<vk::SubpassDependency> &subpassDependencies);

};
} // namespace Vkbase