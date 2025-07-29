#include "Framebuffer.h"

#include "Device.h"
#include "Image.h"
#include "RenderPass.h"

namespace Vkbase {
Framebuffer::Framebuffer(const std::string &resourceName,
                         const std::string &deviceName,
                         const std::string &renderPassName,
                         const std::vector<std::string> &attachmentNames,
                         uint32_t width, uint32_t height)
    : ResourceBase(Vkbase::ResourceType::Framebuffer, resourceName),
      _renderPass(*dynamic_cast<const RenderPass *>(
          connectTo(resourceManager().resource(Vkbase::ResourceType::RenderPass,
                                               renderPassName)))),
      _device(
          *dynamic_cast<const Device *>(connectTo(resourceManager().resource(
              Vkbase::ResourceType::Device, deviceName)))) {
  if (_renderPass.attachmentCount() != attachmentNames.size())
    throw std::runtime_error("The number of image given not enough.");

  std::vector<vk::ImageView> attachments;
  attachments.reserve(attachmentNames.size());
  ResourceManager &manager = resourceManager();

  const std::vector<vk::Format> &_formats = _renderPass.attachmentFormats();

  for (uint32_t i = 0; i < attachmentNames.size(); ++i) {
    const Image &image = *connectTo(dynamic_cast<const Image *>(
        manager.resource(Vkbase::ResourceType::Image, attachmentNames[i])));
    if (image.format() != _formats[i])
      throw std::runtime_error(
          "The format of image(s) different with the RenderPass.");
    attachments.push_back(image.view());
  }

  vk::FramebufferCreateInfo createInfo;
  createInfo.setAttachments(attachments)
      .setRenderPass(_renderPass.renderPass())
      .setLayers(1)
      .setWidth(width)
      .setHeight(height);
  _framebuffer = _device.device().createFramebuffer(createInfo);
}

Framebuffer::~Framebuffer() { _device.device().destroy(_framebuffer); }

const vk::Framebuffer &Framebuffer::framebuffer() const { return _framebuffer; }
}  // namespace Vkbase