#pragma once
#include "ResourceBase.h"

namespace Vkbase {
class Device;
class Framebuffer;
class Pipeline;
struct PipelineCreateInfo;
class RenderPass : public ResourceBase {
 public:
  RenderPass(const std::string &resourceName, const std::string &deviceName,
             const vk::RenderPassCreateInfo &createInfo);
  ~RenderPass() override;
  uint32_t attachmentCount() const;
  const vk::RenderPass &renderPass() const;
  const std::vector<vk::Format> &attachmentFormats() const;
  const Framebuffer &createFramebuffer(
      const std::string &resourceName,
      const std::vector<std::string> &attachmentNames, uint32_t width,
      uint32_t height) const;
  const Pipeline &createPipeline(const std::string &resourceName,
                                 const PipelineCreateInfo &createInfo) const;
  void begin(const vk::CommandBuffer &commandBuffer,
             const Framebuffer &framebuffer,
             std::vector<vk::ClearValue> &clearValues,
             vk::Extent2D &extent) const;
  void end(const vk::CommandBuffer &commandBuffer) const;

 private:
  vk::RenderPass _renderPass;
  const Device &_device;
  uint32_t _attachmentCount;
  std::vector<vk::Format> _attachmentFormats;
};
}  // namespace Vkbase