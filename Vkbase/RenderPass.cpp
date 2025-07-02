#include "RenderPass.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "Device.h"
#include <iostream>

namespace Vkbase
{
    RenderPass::RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo)
        : ResourceBase(Vkbase::ResourceType::RenderPass, resourceName), _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName))))
    {
        _attachmentCount = createInfo.attachmentCount;
        _attachmentFormats.reserve(_attachmentCount);
        for (uint32_t i = 0; i < _attachmentCount; ++i)
            _attachmentFormats.push_back(createInfo.pAttachments[i].format);
        _renderPass = _device.device().createRenderPass(createInfo);
    }

    RenderPass::~RenderPass()
    {
        _device.device().destroy(_renderPass);
    }

    const vk::RenderPass &RenderPass::renderPass() const
    {
        return _renderPass;
    }

    uint32_t RenderPass::attachmentCount() const
    {
        return _attachmentCount;
    }

    const std::vector<vk::Format> &RenderPass::attachmentFormats() const
    {
        return _attachmentFormats;
    }

    const Framebuffer &RenderPass::createFramebuffer(const std::string &resourceName, const std::vector<const std::string> &attachmentNames, uint32_t width, uint32_t height) const
    {
        return *(new Framebuffer(resourceName, _device.name(), name(), attachmentNames, width, height));
    }

    const Pipeline &RenderPass::createPipeline(const std::string &resourceName, const PipelineCreateInfo &createInfo) const
    {
        return *(new Pipeline(resourceName, _device.name(), name(), createInfo));
    }

    void RenderPass::begin(const vk::CommandBuffer &commandBuffer, const Framebuffer &framebuffer, std::vector<vk::ClearValue> &clearValues, vk::Extent2D &extent) const
    {
        vk::RenderPassBeginInfo beginInfo;
        
        vk::Viewport viewport;
        viewport.setX(0.0f)
            .setY(0.0f)
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
        
        vk::Rect2D scissor;
        scissor.setExtent(extent);
        scissor.setOffset({0, 0});

        vk::Rect2D renderArea;
        renderArea.setExtent(extent)
            .setOffset({0, 0});
        beginInfo.setFramebuffer(framebuffer.framebuffer())
            .setRenderPass(_renderPass)
            .setClearValues(clearValues)
            .setRenderArea(renderArea);

        commandBuffer.setViewport(0, viewport);
        commandBuffer.setScissor(0, scissor);

        commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
    }

    void RenderPass::end(const vk::CommandBuffer &commandBuffer) const
    {
        commandBuffer.endRenderPass();
    }
}