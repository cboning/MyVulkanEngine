#include "Framebuffer.h"

#include "Device.h"
#include "Image.h"
#include "RenderPass.h"

namespace Vkbase
{
const vk::Extent2D &Framebuffer::extent() const { return _extent; }

Framebuffer::Framebuffer(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName,
                         const std::vector<std::string> &attachmentNames, uint32_t width, uint32_t height)
    : VkGpuResourceBase(Vkbase::VkResourceType::Framebuffer, resourceName, resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)),
      _renderPass((connectTo(resourceManager().resource(Vkbase::VkResourceType::RenderPass, renderPassName)))), _extent(width, height)
{
    if (auto p = _renderPass.lock<RenderPass>())
        if (p->attachmentCount() != attachmentNames.size())
            throw std::runtime_error("The number of image given not enough.");

    std::vector<vk::ImageView> attachments;
    attachments.reserve(attachmentNames.size());
    VkResourceManager &manager = resourceManager();

    if (auto p = _renderPass.lock<RenderPass>())
    {
        const std::vector<vk::Format> &_formats = p->attachmentFormats();

        for (uint32_t i = 0; i < attachmentNames.size(); ++i)
        {
            const Vkbase::VkResourceManagerHolder::WeakReference image = connectTo(manager.resource(Vkbase::VkResourceType::Image, attachmentNames[i]));
            if (auto p = connectTo(manager.resource(Vkbase::VkResourceType::Image, attachmentNames[i])).lock<Image>())
            {
                if (p->format() != _formats[i])
                    throw std::runtime_error("The format of image(s) different with the RenderPass.");
                attachments.push_back(p->view());
            }
        }
    }

    vk::FramebufferCreateInfo createInfo;
    if (auto p = _renderPass.lock<RenderPass>())
        createInfo.setAttachments(attachments).setRenderPass(p->renderPass()).setLayers(1).setWidth(width).setHeight(height);
    if (auto p = _device.lock<Device>())
        _framebuffer = p->device().createFramebuffer(createInfo);
}

Framebuffer::~Framebuffer()
{
    if (auto p = _device.lock<Device>())
    {
        auto device = p->device();
        auto framebuffer = _framebuffer;

        _onDelayDestroy = [device, framebuffer]() { device.destroy(framebuffer); };
    }
}

const vk::Framebuffer &Framebuffer::framebuffer() const { return _framebuffer; }
} // namespace Vkbase