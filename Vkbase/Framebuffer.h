#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
    class Image;
    class RenderPass;
    class Device;
    class Framebuffer : public ResourceBase
    {
    public:
        Framebuffer(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName, const std::vector<const std::string> &attachmentNames, uint32_t width, uint32_t height);
        ~Framebuffer() override;

    private:
        vk::Framebuffer _framebuffer;
        const RenderPass &_renderPass;
        const Device &_device;

    };
}