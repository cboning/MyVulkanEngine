#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
    class Device;
    class Framebuffer;
    class RenderPass : public ResourceBase
    {
    public:
        RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo);
        ~RenderPass() override;
        const vk::RenderPass &renderPass() const;
        uint32_t attachmentCount() const;
        const std::vector<vk::Format> &attachmentFormats() const;

        const Framebuffer &createFramebuffer(const std::string &resourceName, const std::vector<const std::string> &attachmentNames, uint32_t width, uint32_t height) const;

    private:
        vk::RenderPass _renderPass;
        const Device &_device;
        uint32_t _attachmentCount;

        std::vector<vk::Format> _attachmentFormats;

    };
}