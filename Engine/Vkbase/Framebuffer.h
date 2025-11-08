#pragma once
#include "VkGpuResourceBase.h"

namespace Vkbase
{
class RenderPass;
class Framebuffer : public VkGpuResourceBase
{
    friend class VkResourceManager;
    friend class CommandBuffer;

public:
    const vk::Extent2D &extent() const;
private:
    Framebuffer(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName,
                const std::vector<std::string> &attachmentNames, uint32_t width, uint32_t height);
    ~Framebuffer() override;
    vk::Framebuffer _framebuffer;
    const VkResourceManagerHolder::WeakReference _renderPass;
    const vk::Framebuffer &framebuffer() const;
    vk::Extent2D _extent;
};
} // namespace Vkbase