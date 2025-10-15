#pragma once
#include "GpuResourceBase.h"

namespace Vkbase
{
class Image;
class RenderPass;
class Device;
class Framebuffer : public GpuResourceBase
{
    friend class ResourceManager;
    friend class CommandBuffer;

public:

private:
    Framebuffer(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName,
                const std::vector<std::string> &attachmentNames, uint32_t width, uint32_t height);
    ~Framebuffer() override;
    vk::Framebuffer _framebuffer;
    const RenderPass &_renderPass;
    const vk::Framebuffer &framebuffer() const;
};
} // namespace Vkbase