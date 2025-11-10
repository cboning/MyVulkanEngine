#include "RenderPass.h"
#include "DescriptorSets.h"
#include "Swapchain.h"
#include "Window.h"

#include <iostream>

#include "../JsonConfigReader/JsonConfigReader.h"
#include "CommandBuffer.h"
#include "Device.h"
#include "Framebuffer.h"
#include "Image.h"
#include "Pipeline.h"

namespace Vkbase
{
RenderPass::RenderPass(const std::string &resourceName, const std::string &deviceName, const vk::RenderPassCreateInfo &createInfo)
    : VkGpuResourceBase(Vkbase::VkResourceType::RenderPass, resourceName, resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)),
      _descriptorSets(connectTo(createResource<DescriptorSets>(resourceName, deviceName)))
{
    _attachmentCount = createInfo.attachmentCount;
    _attachmentFormats.reserve(_attachmentCount);
    for (uint32_t i = 0; i < _attachmentCount; ++i)
        _attachmentFormats.push_back(createInfo.pAttachments[i].format);
    if (auto p = _device.lock<Device>())
        _renderPass = p->device().createRenderPass(createInfo);
}

RenderPass::RenderPass(const std::string &resourceName, const std::string &deviceName, const json &config, const std::string &swapchainName = "",
                       vk::Format depthFormat = vk::Format::eUndefined)
    : RenderPass(resourceName, deviceName,
                 JsonConfigReader::getRenderPassCreateInfo(JsonConfigReader::getAttachmentsWithJson(config, swapchainName, depthFormat),
                                                           JsonConfigReader::getSubpassesWithJson(config, JsonConfigReader::getAttachmentRefsWithJson(config)),
                                                           JsonConfigReader::getSubpassDependenciesWithJson(config)))
{
}

RenderPass::~RenderPass()
{
    vk::Device device;
    if (auto p = _device.lock<Device>())
        device = p->device();
    auto renderPass = _renderPass;
    _onDelayDestroy = [device, renderPass]() mutable { device.destroy(renderPass); };
}

const vk::RenderPass &RenderPass::renderPass() const { return _renderPass; }

uint32_t RenderPass::attachmentCount() const { return _attachmentCount; }

const std::vector<vk::Format> &RenderPass::attachmentFormats() const { return _attachmentFormats; }

VkResourceManagerHolder::WeakReference RenderPass::createFramebuffer(const std::string &resourceName, const std::vector<std::string> &attachmentNames,
                                                                     uint32_t width, uint32_t height) const
{
    if (auto p = _device.lock<Device>())
        return createResource<Framebuffer>(resourceName, p->name(), name(), attachmentNames, width, height);
    throw std::runtime_error("Device already destroyed.");
}

std::vector<std::string> RenderPass::createFramebuffer(const std::string &resourceName, const json &config, uint32_t width, uint32_t height,
                                                       const std::string &swapchainName, vk::Format depthFormat) const
{
    std::vector<std::string> framebufferNames;
    uint32_t count = 0;
    {
        const json &countJson = config["count"];
        if (countJson.is_string() && std::string(countJson) == "auto")
        {
            if (auto p = resourceManager().resource(Vkbase::VkResourceType::Swapchain, swapchainName).lock<Swapchain>())
            {
                count = p->imageNames().size();
            }
            else if (countJson.is_number_integer())
            {
                count = countJson;
            }
            else
            {
                throw std::runtime_error("Config Error: The count of framebuffer must be setting.");
            }
        }
    }

    for (uint32_t i = 0; i < count; ++i)
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
                if (auto p = _device.lock<Device>())
                    attachmentNames.push_back(createResource<Vkbase::Image>("Framebuffer_Image_" + std::string(attachment["name"]) + "_" + std::to_string(i),
                                                                            p->name(), imageConfig, nullptr, swapchainName, depthFormat)
                                                  .lock()
                                                  ->name());
            }
            else if (type == "use")
                attachmentNames.push_back(std::string(attachment["name"]) + "_" + std::to_string(i));
        }
        framebufferNames.push_back(createFramebuffer(resourceName + "_" + std::to_string(i), attachmentNames, width, height).lock()->name());
    }
    return framebufferNames;
}

VkResourceManagerHolder::WeakReference RenderPass::createPipeline(const std::string &resourceName, const PipelineCreateInfo &createInfo) const
{

    if (auto p = _device.lock<Device>())
        return createResource<Pipeline>(resourceName, p->name(), name(), createInfo);
    throw std::runtime_error("Device already destroyed.");
}

void RenderPass::createPipelines(const json &config, const std::unordered_map<std::string, VertexInfo> &vertexInfos,
                                 const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> &descriptorSetLayouts,
                                 const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> &viewportInfos)
{
    for (const json &pipelineCreateInfoJson : config)
    {
        const std::string &pipelineName = pipelineCreateInfoJson["name"];
        const std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>> &viewportInfo = viewportInfos.at(pipelineName);
        PipelineRenderInfo renderInfo = PipelineRenderInfo(pipelineCreateInfoJson["renderInfo"], viewportInfo.first, viewportInfo.second);
        std::vector<vk::DescriptorSetLayout> pipelineDescriptorSetLayouts = descriptorSetLayouts.at(pipelineName);

        if (pipelineCreateInfoJson.count("descriptorSetsNames"))
            for (const json &name : pipelineCreateInfoJson["descriptorSetsNames"])
                if (auto p = _descriptorSets.lock<DescriptorSets>())
                    pipelineDescriptorSetLayouts.push_back(p->layout(name));

        createPipeline(pipelineCreateInfoJson["name"], PipelineCreateInfo{ShaderInfo::getShaderInfosWithJson(pipelineCreateInfoJson["shaderInfos"]),
                                                                          vertexInfos.at(pipelineName), pipelineDescriptorSetLayouts, renderInfo});
    }
}

void RenderPass::begin(const VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &framebufferName,
                       const std::vector<vk::ClearValue> &clearValues, vk::SubpassContents subpassContents)
{
    VkResourceManagerHolder::WeakReference framebuffer = resourceManager().resource(Vkbase::VkResourceType::Framebuffer, framebufferName);
    vk::Extent2D extent;
    if (auto p = framebuffer.lock<Framebuffer>())
        extent = p->extent();

    setViewportScissor(commandBuffer, extent);

    vk::Rect2D renderArea;
    renderArea.setExtent(extent).setOffset({0, 0});
    vk::RenderPassBeginInfo beginInfo;
    beginInfo.setClearValues(clearValues).setRenderArea(renderArea);
    auto thisReference = weakReference();
    if (auto pCommandBuffer = commandBuffer.lock<CommandBuffer>())
        pCommandBuffer->beginRenderPass(thisReference, framebuffer, beginInfo, subpassContents);
}

void RenderPass::setViewportScissor(const VkResourceManagerHolder::WeakReference &commandBuffer, const vk::Extent2D &extent)
{
    vk::Viewport viewport;
    viewport.setX(0.0f).setY(0.0f).setWidth(extent.width).setHeight(extent.height).setMinDepth(0.0f).setMaxDepth(1.0f);

    vk::Rect2D scissor;
    scissor.setExtent(extent);
    scissor.setOffset({0, 0});

    if (auto pCommandBuffer = commandBuffer.lock<CommandBuffer>())
    {
        pCommandBuffer->commandBuffer().setViewport(0, viewport);
        pCommandBuffer->commandBuffer().setScissor(0, scissor);
    }
}

void RenderPass::end(const VkResourceManagerHolder::WeakReference &commandBuffer)
{
    if (auto pCommandBuffer = commandBuffer.lock<CommandBuffer>())
        pCommandBuffer->commandBuffer().endRenderPass();
}

VkResourceManagerHolder::WeakReference RenderPass::descriptorSets() const { return _descriptorSets; }

} // namespace Vkbase