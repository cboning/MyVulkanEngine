#include "RenderPass.h"
#include "Swapchain.h"

#include <iostream>

#include "Device.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "Image.h"


namespace Vkbase
{
RenderPass::RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo)
    : ResourceBase(Vkbase::ResourceType::RenderPass, resourceName),
      _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName))))
{
    _attachmentCount = createInfo.attachmentCount;
    _attachmentFormats.reserve(_attachmentCount);
    for (uint32_t i = 0; i < _attachmentCount; ++i)
        _attachmentFormats.push_back(createInfo.pAttachments[i].format);
    _renderPass = _device.device().createRenderPass(createInfo);
}

RenderPass::RenderPass(const std::string &resourceName, const std::string &deviceName, const json &config, const std::string &swapchainName = "", vk::Format depthFormat = vk::Format::eUndefined)
    : RenderPass(resourceName, deviceName, JsonConfigReader::getRenderPassCreateInfo(JsonConfigReader::getAttachmentsWithJson(config, swapchainName, depthFormat), JsonConfigReader::getSubpassesWithJson(config, JsonConfigReader::getAttachmentRefsWithJson(config)), JsonConfigReader::getSubpassDependenciesWithJson(config)))
{

}

RenderPass::~RenderPass() { _device.device().destroy(_renderPass); }

const vk::RenderPass &RenderPass::renderPass() const { return _renderPass; }

uint32_t RenderPass::attachmentCount() const { return _attachmentCount; }

const std::vector<vk::Format> &RenderPass::attachmentFormats() const { return _attachmentFormats; }

const Framebuffer &RenderPass::createFramebuffer(const std::string &resourceName, const std::vector<std::string> &attachmentNames, uint32_t width,
                                                 uint32_t height) const
{
    return *resourceManager().create<Framebuffer>(resourceName, _device.name(), name(), attachmentNames, width, height);
}

std::vector<std::string> RenderPass::createFramebuffer(const std::string &resourceName, const json &config, uint32_t width, uint32_t height, const std::string &swapchainName, vk::Format depthFormat) const
{
    std::vector<std::string> framebufferNames;
    for (uint32_t i = 0; i < config["count"]; ++i)
    {
        std::vector<std::string> attachmentNames;
        for (const json &attachment : config["images"])
        {
            std::string type = attachment["type"];
            if (type == "create")
            {
                json imageConfig = attachment["imageConfig"];
                imageConfig["width"] = width;
                imageConfig["height"] = height;
                imageConfig["depth"] = 1;
                attachmentNames.push_back(resourceManager().create<Vkbase::Image>("Framebuffer_Image_" + std::string(attachment["name"]) + "_" + std::to_string(i), _device.name(), imageConfig, nullptr, swapchainName, depthFormat)->name());
            }
            else if (type == "use")
                attachmentNames.push_back(std::string(attachment["name"]) + "_" + std::to_string(i));
        }
        framebufferNames.push_back(createFramebuffer(resourceName + "_" + std::to_string(i), attachmentNames, width, height).name());
    }
    return framebufferNames;
}

const Pipeline &RenderPass::createPipeline(const std::string &resourceName, const PipelineCreateInfo &createInfo) const
{
    return *resourceManager().create<Pipeline>(resourceName, _device.name(), name(), createInfo);
}

void RenderPass::begin(const vk::CommandBuffer &commandBuffer, const Framebuffer &framebuffer, std::vector<vk::ClearValue> &clearValues,
                       vk::Extent2D &extent) const
{
    vk::RenderPassBeginInfo beginInfo;

    vk::Viewport viewport;
    viewport.setX(0.0f).setY(0.0f).setWidth(extent.width).setHeight(extent.height).setMinDepth(0.0f).setMaxDepth(1.0f);

    vk::Rect2D scissor;
    scissor.setExtent(extent);
    scissor.setOffset({0, 0});

    vk::Rect2D renderArea;
    renderArea.setExtent(extent).setOffset({0, 0});
    beginInfo.setFramebuffer(framebuffer.framebuffer()).setRenderPass(_renderPass).setClearValues(clearValues).setRenderArea(renderArea);

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
}

void RenderPass::end(const vk::CommandBuffer &commandBuffer) const { commandBuffer.endRenderPass(); }


} // namespace Vkbase