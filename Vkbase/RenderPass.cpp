#include "RenderPass.h"
#include "Swapchain.h"

#include <iostream>

#include "Device.h"
#include "Framebuffer.h"
#include "Pipeline.h"

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
    : RenderPass(resourceName, deviceName, getRenderPassCreateInfo(getAttachmentsWithJson(config, swapchainName, depthFormat), getSubpassesWithJson(config, getAttachmentRefsWithJson(config)), getSubpassDependenciesWithJson(config)))
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

vk::Format RenderPass::getFormatWithJson(const std::string &format, const std::string &swapchainName, vk::Format depthFormat)
{
    static const std::unordered_map<std::string, vk::Format> formatMap = {
        {"Undefined", vk::Format::eUndefined},
        {"R4G4UnormPack8", vk::Format::eR4G4UnormPack8},
        {"R4G4B4A4UnormPack16", vk::Format::eR4G4B4A4UnormPack16},
        {"B4G4R4A4UnormPack16", vk::Format::eB4G4R4A4UnormPack16},
        {"R5G6B5UnormPack16", vk::Format::eR5G6B5UnormPack16},
        {"B5G6R5UnormPack16", vk::Format::eB5G6R5UnormPack16},
        {"R5G5B5A1UnormPack16", vk::Format::eR5G5B5A1UnormPack16},
        {"B5G5R5A1UnormPack16", vk::Format::eB5G5R5A1UnormPack16},
        {"A1R5G5B5UnormPack16", vk::Format::eA1R5G5B5UnormPack16},
        {"R8Unorm", vk::Format::eR8Unorm},
        {"R8Snorm", vk::Format::eR8Snorm},
        {"R8Uscaled", vk::Format::eR8Uscaled},
        {"R8Sscaled", vk::Format::eR8Sscaled},
        {"R8Uint", vk::Format::eR8Uint},
        {"R8Sint", vk::Format::eR8Sint},
        {"R8Srgb", vk::Format::eR8Srgb},
        {"R8G8Unorm", vk::Format::eR8G8Unorm},
        {"R8G8Snorm", vk::Format::eR8G8Snorm},
        {"R8G8Uscaled", vk::Format::eR8G8Uscaled},
        {"R8G8Sscaled", vk::Format::eR8G8Sscaled},
        {"R8G8Uint", vk::Format::eR8G8Uint},
        {"R8G8Sint", vk::Format::eR8G8Sint},
        {"R8G8Srgb", vk::Format::eR8G8Srgb},
        {"R8G8B8Unorm", vk::Format::eR8G8B8Unorm},
        {"R8G8B8Snorm", vk::Format::eR8G8B8Snorm},
        {"R8G8B8Uscaled", vk::Format::eR8G8B8Uscaled},
        {"R8G8B8Sscaled", vk::Format::eR8G8B8Sscaled},
        {"R8G8B8Uint", vk::Format::eR8G8B8Uint},
        {"R8G8B8Sint", vk::Format::eR8G8B8Sint},
        {"R8G8B8Srgb", vk::Format::eR8G8B8Srgb},
        {"B8G8R8Unorm", vk::Format::eB8G8R8Unorm},
        {"B8G8R8Snorm", vk::Format::eB8G8R8Snorm},
        {"B8G8R8Uscaled", vk::Format::eB8G8R8Uscaled},
        {"B8G8R8Sscaled", vk::Format::eB8G8R8Sscaled},
        {"B8G8R8Uint", vk::Format::eB8G8R8Uint},
        {"B8G8R8Sint", vk::Format::eB8G8R8Sint},
        {"B8G8R8Srgb", vk::Format::eB8G8R8Srgb},
        {"R8G8B8A8Unorm", vk::Format::eR8G8B8A8Unorm},
        {"R8G8B8A8Snorm", vk::Format::eR8G8B8A8Snorm},
        {"R8G8B8A8Uscaled", vk::Format::eR8G8B8A8Uscaled},
        {"R8G8B8A8Sscaled", vk::Format::eR8G8B8A8Sscaled},
        {"R8G8B8A8Uint", vk::Format::eR8G8B8A8Uint},
        {"R8G8B8A8Sint", vk::Format::eR8G8B8A8Sint},
        {"R8G8B8A8Srgb", vk::Format::eR8G8B8A8Srgb},
        {"B8G8R8A8Unorm", vk::Format::eB8G8R8A8Unorm},
        {"B8G8R8A8Snorm", vk::Format::eB8G8R8A8Snorm},
        {"B8G8R8A8Uscaled", vk::Format::eB8G8R8A8Uscaled},
        {"B8G8R8A8Sscaled", vk::Format::eB8G8R8A8Sscaled},
        {"B8G8R8A8Uint", vk::Format::eB8G8R8A8Uint},
        {"B8G8R8A8Sint", vk::Format::eB8G8R8A8Sint},
        {"B8G8R8A8Srgb", vk::Format::eB8G8R8A8Srgb},
        {"A8B8G8R8UnormPack32", vk::Format::eA8B8G8R8UnormPack32},
        {"A8B8G8R8SnormPack32", vk::Format::eA8B8G8R8SnormPack32},
        {"A8B8G8R8UscaledPack32", vk::Format::eA8B8G8R8UscaledPack32},
        {"A8B8G8R8SscaledPack32", vk::Format::eA8B8G8R8SscaledPack32},
        {"A8B8G8R8UintPack32", vk::Format::eA8B8G8R8UintPack32},
        {"A8B8G8R8SintPack32", vk::Format::eA8B8G8R8SintPack32},
        {"A8B8G8R8SrgbPack32", vk::Format::eA8B8G8R8SrgbPack32},
        {"A2R10G10B10UnormPack32", vk::Format::eA2R10G10B10UnormPack32},
        {"A2R10G10B10SnormPack32", vk::Format::eA2R10G10B10SnormPack32},
        {"A2R10G10B10UscaledPack32", vk::Format::eA2R10G10B10UscaledPack32},
        {"A2R10G10B10SscaledPack32", vk::Format::eA2R10G10B10SscaledPack32},
        {"A2R10G10B10UintPack32", vk::Format::eA2R10G10B10UintPack32},
        {"A2R10G10B10SintPack32", vk::Format::eA2R10G10B10SintPack32},
        {"A2B10G10R10UnormPack32", vk::Format::eA2B10G10R10UnormPack32},
        {"A2B10G10R10SnormPack32", vk::Format::eA2B10G10R10SnormPack32},
        {"A2B10G10R10UscaledPack32", vk::Format::eA2B10G10R10UscaledPack32},
        {"A2B10G10R10SscaledPack32", vk::Format::eA2B10G10R10SscaledPack32},
        {"A2B10G10R10UintPack32", vk::Format::eA2B10G10R10UintPack32},
        {"A2B10G10R10SintPack32", vk::Format::eA2B10G10R10SintPack32},
        {"R16Unorm", vk::Format::eR16Unorm},
        {"R16Snorm", vk::Format::eR16Snorm},
        {"R16Uscaled", vk::Format::eR16Uscaled},
        {"R16Sscaled", vk::Format::eR16Sscaled},
        {"R16Uint", vk::Format::eR16Uint},
        {"R16Sint", vk::Format::eR16Sint},
        {"R16Sfloat", vk::Format::eR16Sfloat},
        {"R16G16Unorm", vk::Format::eR16G16Unorm},
        {"R16G16Snorm", vk::Format::eR16G16Snorm},
        {"R16G16Uscaled", vk::Format::eR16G16Uscaled},
        {"R16G16Sscaled", vk::Format::eR16G16Sscaled},
        {"R16G16Uint", vk::Format::eR16G16Uint},
        {"R16G16Sint", vk::Format::eR16G16Sint},
        {"R16G16Sfloat", vk::Format::eR16G16Sfloat},
        {"R16G16B16Unorm", vk::Format::eR16G16B16Unorm},
        {"R16G16B16Snorm", vk::Format::eR16G16B16Snorm},
        {"R16G16B16Uscaled", vk::Format::eR16G16B16Uscaled},
        {"R16G16B16Sscaled", vk::Format::eR16G16B16Sscaled},
        {"R16G16B16Uint", vk::Format::eR16G16B16Uint},
        {"R16G16B16Sint", vk::Format::eR16G16B16Sint},
        {"R16G16B16Sfloat", vk::Format::eR16G16B16Sfloat},
        {"R16G16B16A16Unorm", vk::Format::eR16G16B16A16Unorm},
        {"R16G16B16A16Snorm", vk::Format::eR16G16B16A16Snorm},
        {"R16G16B16A16Uscaled", vk::Format::eR16G16B16A16Uscaled},
        {"R16G16B16A16Sscaled", vk::Format::eR16G16B16A16Sscaled},
        {"R16G16B16A16Uint", vk::Format::eR16G16B16A16Uint},
        {"R16G16B16A16Sint", vk::Format::eR16G16B16A16Sint},
        {"R16G16B16A16Sfloat", vk::Format::eR16G16B16A16Sfloat},
        {"R32Uint", vk::Format::eR32Uint},
        {"R32Sint", vk::Format::eR32Sint},
        {"R32Sfloat", vk::Format::eR32Sfloat},
        {"R32G32Uint", vk::Format::eR32G32Uint},
        {"R32G32Sint", vk::Format::eR32G32Sint},
        {"R32G32Sfloat", vk::Format::eR32G32Sfloat},
        {"R32G32B32Uint", vk::Format::eR32G32B32Uint},
        {"R32G32B32Sint", vk::Format::eR32G32B32Sint},
        {"R32G32B32Sfloat", vk::Format::eR32G32B32Sfloat},
        {"R32G32B32A32Uint", vk::Format::eR32G32B32A32Uint},
        {"R32G32B32A32Sint", vk::Format::eR32G32B32A32Sint},
        {"R32G32B32A32Sfloat", vk::Format::eR32G32B32A32Sfloat},
        {"R64Uint", vk::Format::eR64Uint},
        {"R64Sint", vk::Format::eR64Sint},
        {"R64Sfloat", vk::Format::eR64Sfloat},
        {"R64G64Uint", vk::Format::eR64G64Uint},
        {"R64G64Sint", vk::Format::eR64G64Sint},
        {"R64G64Sfloat", vk::Format::eR64G64Sfloat},
        {"R64G64B64Uint", vk::Format::eR64G64B64Uint},
        {"R64G64B64Sint", vk::Format::eR64G64B64Sint},
        {"R64G64B64Sfloat", vk::Format::eR64G64B64Sfloat},
        {"R64G64B64A64Uint", vk::Format::eR64G64B64A64Uint},
        {"R64G64B64A64Sint", vk::Format::eR64G64B64A64Sint},
        {"R64G64B64A64Sfloat", vk::Format::eR64G64B64A64Sfloat},
        {"B10G11R11UfloatPack32", vk::Format::eB10G11R11UfloatPack32},
        {"E5B9G9R9UfloatPack32", vk::Format::eE5B9G9R9UfloatPack32},
        {"D16Unorm", vk::Format::eD16Unorm},
        {"X8D24UnormPack32", vk::Format::eX8D24UnormPack32},
        {"D32Sfloat", vk::Format::eD32Sfloat},
        {"S8Uint", vk::Format::eS8Uint},
        {"D16UnormS8Uint", vk::Format::eD16UnormS8Uint},
        {"D24UnormS8Uint", vk::Format::eD24UnormS8Uint},
        {"D32SfloatS8Uint", vk::Format::eD32SfloatS8Uint},
        {"Bc1RgbUnormBlock", vk::Format::eBc1RgbUnormBlock},
        {"Bc1RgbSrgbBlock", vk::Format::eBc1RgbSrgbBlock},
        {"Bc1RgbaUnormBlock", vk::Format::eBc1RgbaUnormBlock},
        {"Bc1RgbaSrgbBlock", vk::Format::eBc1RgbaSrgbBlock},
        {"Bc2UnormBlock", vk::Format::eBc2UnormBlock},
        {"Bc2SrgbBlock", vk::Format::eBc2SrgbBlock},
        {"Bc3UnormBlock", vk::Format::eBc3UnormBlock},
        {"Bc3SrgbBlock", vk::Format::eBc3SrgbBlock},
        {"Bc4UnormBlock", vk::Format::eBc4UnormBlock},
        {"Bc4SnormBlock", vk::Format::eBc4SnormBlock},
        {"Bc5UnormBlock", vk::Format::eBc5UnormBlock},
        {"Bc5SnormBlock", vk::Format::eBc5SnormBlock},
        {"Bc6HUfloatBlock", vk::Format::eBc6HUfloatBlock},
        {"Bc6HSfloatBlock", vk::Format::eBc6HSfloatBlock},
        {"Bc7UnormBlock", vk::Format::eBc7UnormBlock},
        {"Bc7SrgbBlock", vk::Format::eBc7SrgbBlock},
        {"Etc2R8G8B8UnormBlock", vk::Format::eEtc2R8G8B8UnormBlock},
        {"Etc2R8G8B8SrgbBlock", vk::Format::eEtc2R8G8B8SrgbBlock},
        {"Etc2R8G8B8A1UnormBlock", vk::Format::eEtc2R8G8B8A1UnormBlock},
        {"Etc2R8G8B8A1SrgbBlock", vk::Format::eEtc2R8G8B8A1SrgbBlock},
        {"Etc2R8G8B8A8UnormBlock", vk::Format::eEtc2R8G8B8A8UnormBlock},
        {"Etc2R8G8B8A8SrgbBlock", vk::Format::eEtc2R8G8B8A8SrgbBlock},
        {"EacR11UnormBlock", vk::Format::eEacR11UnormBlock},
        {"EacR11SnormBlock", vk::Format::eEacR11SnormBlock},
        {"EacR11G11UnormBlock", vk::Format::eEacR11G11UnormBlock},
        {"EacR11G11SnormBlock", vk::Format::eEacR11G11SnormBlock},
        {"Astc4x4UnormBlock", vk::Format::eAstc4x4UnormBlock},
        {"Astc4x4SrgbBlock", vk::Format::eAstc4x4SrgbBlock},
        {"Astc5x4UnormBlock", vk::Format::eAstc5x4UnormBlock},
        {"Astc5x4SrgbBlock", vk::Format::eAstc5x4SrgbBlock},
        {"Astc5x5UnormBlock", vk::Format::eAstc5x5UnormBlock},
        {"Astc5x5SrgbBlock", vk::Format::eAstc5x5SrgbBlock},
        {"Astc6x5UnormBlock", vk::Format::eAstc6x5UnormBlock},
        {"Astc6x5SrgbBlock", vk::Format::eAstc6x5SrgbBlock},
        {"Astc6x6UnormBlock", vk::Format::eAstc6x6UnormBlock},
        {"Astc6x6SrgbBlock", vk::Format::eAstc6x6SrgbBlock},
        {"Astc8x5UnormBlock", vk::Format::eAstc8x5UnormBlock},
        {"Astc8x5SrgbBlock", vk::Format::eAstc8x5SrgbBlock},
        {"Astc8x6UnormBlock", vk::Format::eAstc8x6UnormBlock},
        {"Astc8x6SrgbBlock", vk::Format::eAstc8x6SrgbBlock},
        {"Astc8x8UnormBlock", vk::Format::eAstc8x8UnormBlock},
        {"Astc8x8SrgbBlock", vk::Format::eAstc8x8SrgbBlock},
        {"Astc10x5UnormBlock", vk::Format::eAstc10x5UnormBlock},
        {"Astc10x5SrgbBlock", vk::Format::eAstc10x5SrgbBlock},
        {"Astc10x6UnormBlock", vk::Format::eAstc10x6UnormBlock},
        {"Astc10x6SrgbBlock", vk::Format::eAstc10x6SrgbBlock},
        {"Astc10x8UnormBlock", vk::Format::eAstc10x8UnormBlock},
        {"Astc10x8SrgbBlock", vk::Format::eAstc10x8SrgbBlock},
        {"Astc10x10UnormBlock", vk::Format::eAstc10x10UnormBlock},
        {"Astc10x10SrgbBlock", vk::Format::eAstc10x10SrgbBlock},
        {"Astc12x10UnormBlock", vk::Format::eAstc12x10UnormBlock},
        {"Astc12x10SrgbBlock", vk::Format::eAstc12x10SrgbBlock},
        {"Astc12x12UnormBlock", vk::Format::eAstc12x12UnormBlock},
        {"Astc12x12SrgbBlock", vk::Format::eAstc12x12SrgbBlock},
        {"G8B8G8R8422Unorm", vk::Format::eG8B8G8R8422Unorm},
        {"G8B8G8R8422UnormKHR", vk::Format::eG8B8G8R8422UnormKHR},
        {"B8G8R8G8422Unorm", vk::Format::eB8G8R8G8422Unorm},
        {"B8G8R8G8422UnormKHR", vk::Format::eB8G8R8G8422UnormKHR},
        {"G8B8R83Plane420Unorm", vk::Format::eG8B8R83Plane420Unorm},
        {"G8B8R83Plane420UnormKHR", vk::Format::eG8B8R83Plane420UnormKHR},
        {"G8B8R82Plane420Unorm", vk::Format::eG8B8R82Plane420Unorm},
        {"G8B8R82Plane420UnormKHR", vk::Format::eG8B8R82Plane420UnormKHR},
        {"G8B8R83Plane422Unorm", vk::Format::eG8B8R83Plane422Unorm},
        {"G8B8R83Plane422UnormKHR", vk::Format::eG8B8R83Plane422UnormKHR},
        {"G8B8R82Plane422Unorm", vk::Format::eG8B8R82Plane422Unorm},
        {"G8B8R82Plane422UnormKHR", vk::Format::eG8B8R82Plane422UnormKHR},
        {"G8B8R83Plane444Unorm", vk::Format::eG8B8R83Plane444Unorm},
        {"G8B8R83Plane444UnormKHR", vk::Format::eG8B8R83Plane444UnormKHR},
        {"R10X6UnormPack16", vk::Format::eR10X6UnormPack16},
        {"R10X6UnormPack16KHR", vk::Format::eR10X6UnormPack16KHR},
        {"R10X6G10X6Unorm2Pack16", vk::Format::eR10X6G10X6Unorm2Pack16},
        {"R10X6G10X6Unorm2Pack16KHR", vk::Format::eR10X6G10X6Unorm2Pack16KHR},
        {"R10X6G10X6B10X6A10X6Unorm4Pack16", vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16},
        {"R10X6G10X6B10X6A10X6Unorm4Pack16KHR", vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16KHR},
        {"G10X6B10X6G10X6R10X6422Unorm4Pack16", vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16},
        {"G10X6B10X6G10X6R10X6422Unorm4Pack16KHR", vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16KHR},
        {"B10X6G10X6R10X6G10X6422Unorm4Pack16", vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16},
        {"B10X6G10X6R10X6G10X6422Unorm4Pack16KHR", vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16KHR},
        {"G10X6B10X6R10X63Plane420Unorm3Pack16", vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16},
        {"G10X6B10X6R10X63Plane420Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16KHR},
        {"G10X6B10X6R10X62Plane420Unorm3Pack16", vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16},
        {"G10X6B10X6R10X62Plane420Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16KHR},
        {"G10X6B10X6R10X63Plane422Unorm3Pack16", vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16},
        {"G10X6B10X6R10X63Plane422Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16KHR},
        {"G10X6B10X6R10X62Plane422Unorm3Pack16", vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16},
        {"G10X6B10X6R10X62Plane422Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16KHR},
        {"G10X6B10X6R10X63Plane444Unorm3Pack16", vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16},
        {"G10X6B10X6R10X63Plane444Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16KHR},
        {"R12X4UnormPack16", vk::Format::eR12X4UnormPack16},
        {"R12X4UnormPack16KHR", vk::Format::eR12X4UnormPack16KHR},
        {"R12X4G12X4Unorm2Pack16", vk::Format::eR12X4G12X4Unorm2Pack16},
        {"R12X4G12X4Unorm2Pack16KHR", vk::Format::eR12X4G12X4Unorm2Pack16KHR},
        {"R12X4G12X4B12X4A12X4Unorm4Pack16", vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16},
        {"R12X4G12X4B12X4A12X4Unorm4Pack16KHR", vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16KHR},
        {"G12X4B12X4G12X4R12X4422Unorm4Pack16", vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16},
        {"G12X4B12X4G12X4R12X4422Unorm4Pack16KHR", vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16KHR},
        {"B12X4G12X4R12X4G12X4422Unorm4Pack16", vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16},
        {"B12X4G12X4R12X4G12X4422Unorm4Pack16KHR", vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16KHR},
        {"G12X4B12X4R12X43Plane420Unorm3Pack16", vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16},
        {"G12X4B12X4R12X43Plane420Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16KHR},
        {"G12X4B12X4R12X42Plane420Unorm3Pack16", vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16},
        {"G12X4B12X4R12X42Plane420Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16KHR},
        {"G12X4B12X4R12X43Plane422Unorm3Pack16", vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16},
        {"G12X4B12X4R12X43Plane422Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16KHR},
        {"G12X4B12X4R12X42Plane422Unorm3Pack16", vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16},
        {"G12X4B12X4R12X42Plane422Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16KHR},
        {"G12X4B12X4R12X43Plane444Unorm3Pack16", vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16},
        {"G12X4B12X4R12X43Plane444Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16KHR},
        {"G16B16G16R16422Unorm", vk::Format::eG16B16G16R16422Unorm},
        {"G16B16G16R16422UnormKHR", vk::Format::eG16B16G16R16422UnormKHR},
        {"B16G16R16G16422Unorm", vk::Format::eB16G16R16G16422Unorm},
        {"B16G16R16G16422UnormKHR", vk::Format::eB16G16R16G16422UnormKHR},
        {"G16B16R163Plane420Unorm", vk::Format::eG16B16R163Plane420Unorm},
        {"G16B16R163Plane420UnormKHR", vk::Format::eG16B16R163Plane420UnormKHR},
        {"G16B16R162Plane420Unorm", vk::Format::eG16B16R162Plane420Unorm},
        {"G16B16R162Plane420UnormKHR", vk::Format::eG16B16R162Plane420UnormKHR},
        {"G16B16R163Plane422Unorm", vk::Format::eG16B16R163Plane422Unorm},
        {"G16B16R163Plane422UnormKHR", vk::Format::eG16B16R163Plane422UnormKHR},
        {"G16B16R162Plane422Unorm", vk::Format::eG16B16R162Plane422Unorm},
        {"G16B16R162Plane422UnormKHR", vk::Format::eG16B16R162Plane422UnormKHR},
        {"G16B16R163Plane444Unorm", vk::Format::eG16B16R163Plane444Unorm},
        {"G16B16R163Plane444UnormKHR", vk::Format::eG16B16R163Plane444UnormKHR},
        {"G8B8R82Plane444Unorm", vk::Format::eG8B8R82Plane444Unorm},
        {"G8B8R82Plane444UnormEXT", vk::Format::eG8B8R82Plane444UnormEXT},
        {"G10X6B10X6R10X62Plane444Unorm3Pack16", vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16},
        {"G10X6B10X6R10X62Plane444Unorm3Pack16EXT", vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16EXT},
        {"G12X4B12X4R12X42Plane444Unorm3Pack16", vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16},
        {"G12X4B12X4R12X42Plane444Unorm3Pack16EXT", vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16EXT},
        {"G16B16R162Plane444Unorm", vk::Format::eG16B16R162Plane444Unorm},
        {"G16B16R162Plane444UnormEXT", vk::Format::eG16B16R162Plane444UnormEXT},
        {"A4R4G4B4UnormPack16", vk::Format::eA4R4G4B4UnormPack16},
        {"A4R4G4B4UnormPack16EXT", vk::Format::eA4R4G4B4UnormPack16EXT},
        {"A4B4G4R4UnormPack16", vk::Format::eA4B4G4R4UnormPack16},
        {"A4B4G4R4UnormPack16EXT", vk::Format::eA4B4G4R4UnormPack16EXT},
        {"Astc4x4SfloatBlock", vk::Format::eAstc4x4SfloatBlock},
        {"Astc4x4SfloatBlockEXT", vk::Format::eAstc4x4SfloatBlockEXT},
        {"Astc5x4SfloatBlock", vk::Format::eAstc5x4SfloatBlock},
        {"Astc5x4SfloatBlockEXT", vk::Format::eAstc5x4SfloatBlockEXT},
        {"Astc5x5SfloatBlock", vk::Format::eAstc5x5SfloatBlock},
        {"Astc5x5SfloatBlockEXT", vk::Format::eAstc5x5SfloatBlockEXT},
        {"Astc6x5SfloatBlock", vk::Format::eAstc6x5SfloatBlock},
        {"Astc6x5SfloatBlockEXT", vk::Format::eAstc6x5SfloatBlockEXT},
        {"Astc6x6SfloatBlock", vk::Format::eAstc6x6SfloatBlock},
        {"Astc6x6SfloatBlockEXT", vk::Format::eAstc6x6SfloatBlockEXT},
        {"Astc8x5SfloatBlock", vk::Format::eAstc8x5SfloatBlock},
        {"Astc8x5SfloatBlockEXT", vk::Format::eAstc8x5SfloatBlockEXT},
        {"Astc8x6SfloatBlock", vk::Format::eAstc8x6SfloatBlock},
        {"Astc8x6SfloatBlockEXT", vk::Format::eAstc8x6SfloatBlockEXT},
        {"Astc8x8SfloatBlock", vk::Format::eAstc8x8SfloatBlock},
        {"Astc8x8SfloatBlockEXT", vk::Format::eAstc8x8SfloatBlockEXT},
        {"Astc10x5SfloatBlock", vk::Format::eAstc10x5SfloatBlock},
        {"Astc10x5SfloatBlockEXT", vk::Format::eAstc10x5SfloatBlockEXT},
        {"Astc10x6SfloatBlock", vk::Format::eAstc10x6SfloatBlock},
        {"Astc10x6SfloatBlockEXT", vk::Format::eAstc10x6SfloatBlockEXT},
        {"Astc10x8SfloatBlock", vk::Format::eAstc10x8SfloatBlock},
        {"Astc10x8SfloatBlockEXT", vk::Format::eAstc10x8SfloatBlockEXT},
        {"Astc10x10SfloatBlock", vk::Format::eAstc10x10SfloatBlock},
        {"Astc10x10SfloatBlockEXT", vk::Format::eAstc10x10SfloatBlockEXT},
        {"Astc12x10SfloatBlock", vk::Format::eAstc12x10SfloatBlock},
        {"Astc12x10SfloatBlockEXT", vk::Format::eAstc12x10SfloatBlockEXT},
        {"Astc12x12SfloatBlock", vk::Format::eAstc12x12SfloatBlock},
        {"Astc12x12SfloatBlockEXT", vk::Format::eAstc12x12SfloatBlockEXT},
        {"Pvrtc12BppUnormBlockIMG", vk::Format::ePvrtc12BppUnormBlockIMG},
        {"Pvrtc14BppUnormBlockIMG", vk::Format::ePvrtc14BppUnormBlockIMG},
        {"Pvrtc22BppUnormBlockIMG", vk::Format::ePvrtc22BppUnormBlockIMG},
        {"Pvrtc24BppUnormBlockIMG", vk::Format::ePvrtc24BppUnormBlockIMG},
        {"Pvrtc12BppSrgbBlockIMG", vk::Format::ePvrtc12BppSrgbBlockIMG},
        {"Pvrtc14BppSrgbBlockIMG", vk::Format::ePvrtc14BppSrgbBlockIMG},
        {"Pvrtc22BppSrgbBlockIMG", vk::Format::ePvrtc22BppSrgbBlockIMG},
        {"Pvrtc24BppSrgbBlockIMG", vk::Format::ePvrtc24BppSrgbBlockIMG},
        {"R16G16Sfixed5NV", vk::Format::eR16G16Sfixed5NV},
        {"R16G16S105NV", vk::Format::eR16G16S105NV},
        {"A1B5G5R5UnormPack16KHR", vk::Format::eA1B5G5R5UnormPack16KHR},
        {"A8UnormKHR", vk::Format::eA8UnormKHR}};
    
    auto it = formatMap.find(format);
    if (it != formatMap.end())
        return it->second;
    else
    {
        if (format == "SWAPCHAIN_FORMAT")
            return dynamic_cast<Vkbase::Swapchain *>(resourceManager().resource(Vkbase::ResourceType::Swapchain, swapchainName))->format();
        else if (format == "DEPTH_FORMAT")
            return depthFormat;
        else
            throw std::invalid_argument("Unknown Format: " + format);
    }
}

vk::SampleCountFlagBits RenderPass::getSampleCountFlagBitWithJson(uint32_t sampleCount) { return (vk::SampleCountFlagBits)sampleCount; }

vk::AttachmentLoadOp RenderPass::getAttachmentLoadOpWithJson(const std::string &loadOp)
{
    static const std::unordered_map<std::string, vk::AttachmentLoadOp> loadOpMap = {{"Load", vk::AttachmentLoadOp::eLoad},
                                                                                    {"Clear", vk::AttachmentLoadOp::eClear},
                                                                                    {"DontCare", vk::AttachmentLoadOp::eDontCare},
                                                                                    {"NoneKHR", vk::AttachmentLoadOp::eNoneKHR},
                                                                                    {"NoneEXT", vk::AttachmentLoadOp::eNoneEXT}};
    auto it = loadOpMap.find(loadOp);
    if (it != loadOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown AttachmentLoadOp: " + loadOp);
}

vk::AttachmentStoreOp RenderPass::getAttachmentStoreOpWithJson(const std::string &loadOp)
{
    static const std::unordered_map<std::string, vk::AttachmentStoreOp> loadOpMap = {
        {"Store", vk::AttachmentStoreOp::eStore},     {"DontCare", vk::AttachmentStoreOp::eDontCare}, {"None", vk::AttachmentStoreOp::eNone},
        {"NoneEXT", vk::AttachmentStoreOp::eNoneEXT}, {"NoneKHR", vk::AttachmentStoreOp::eNoneKHR},   {"NoneQCOM", vk::AttachmentStoreOp::eNoneQCOM},
    };
    auto it = loadOpMap.find(loadOp);
    if (it != loadOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown AttachmentStoreOp: " + loadOp);
}

vk::ImageLayout RenderPass::getImageLayoutWithJson(const std::string &imageLayout)
{
    static const std::unordered_map<std::string, vk::ImageLayout> imageLayoutMap = {
        {"Undefined", vk::ImageLayout::eUndefined},
        {"General", vk::ImageLayout::eGeneral},
        {"ColorAttachmentOptimal", vk::ImageLayout::eColorAttachmentOptimal},
        {"DepthStencilAttachmentOptimal", vk::ImageLayout::eDepthStencilAttachmentOptimal},
        {"DepthStencilReadOnlyOptimal", vk::ImageLayout::eDepthStencilReadOnlyOptimal},
        {"ShaderReadOnlyOptimal", vk::ImageLayout::eShaderReadOnlyOptimal},
        {"TransferSrcOptimal", vk::ImageLayout::eTransferSrcOptimal},
        {"TransferDstOptimal", vk::ImageLayout::eTransferDstOptimal},
        {"Preinitialized", vk::ImageLayout::ePreinitialized},
        {"DepthReadOnlyStencilAttachmentOptimal", vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal},
        {"DepthReadOnlyStencilAttachmentOptimalKHR", vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimalKHR},
        {"DepthAttachmentStencilReadOnlyOptimal", vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal},
        {"DepthAttachmentStencilReadOnlyOptimalKHR", vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimalKHR},
        {"DepthAttachmentOptimal", vk::ImageLayout::eDepthAttachmentOptimal},
        {"DepthAttachmentOptimalKHR", vk::ImageLayout::eDepthAttachmentOptimalKHR},
        {"DepthReadOnlyOptimal", vk::ImageLayout::eDepthReadOnlyOptimal},
        {"DepthReadOnlyOptimalKHR", vk::ImageLayout::eDepthReadOnlyOptimalKHR},
        {"StencilAttachmentOptimal", vk::ImageLayout::eStencilAttachmentOptimal},
        {"StencilAttachmentOptimalKHR", vk::ImageLayout::eStencilAttachmentOptimalKHR},
        {"StencilReadOnlyOptimal", vk::ImageLayout::eStencilReadOnlyOptimal},
        {"StencilReadOnlyOptimalKHR", vk::ImageLayout::eStencilReadOnlyOptimalKHR},
        {"ReadOnlyOptimal", vk::ImageLayout::eReadOnlyOptimal},
        {"ReadOnlyOptimalKHR", vk::ImageLayout::eReadOnlyOptimalKHR},
        {"AttachmentOptimal", vk::ImageLayout::eAttachmentOptimal},
        {"AttachmentOptimalKHR", vk::ImageLayout::eAttachmentOptimalKHR},
        {"PresentSrcKHR", vk::ImageLayout::ePresentSrcKHR},
        {"VideoDecodeDstKHR", vk::ImageLayout::eVideoDecodeDstKHR},
        {"VideoDecodeSrcKHR", vk::ImageLayout::eVideoDecodeSrcKHR},
        {"VideoDecodeDpbKHR", vk::ImageLayout::eVideoDecodeDpbKHR},
        {"SharedPresentKHR", vk::ImageLayout::eSharedPresentKHR},
        {"FragmentDensityMapOptimalEXT", vk::ImageLayout::eFragmentDensityMapOptimalEXT},
        {"FragmentShadingRateAttachmentOptimalKHR", vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR},
        {"ShadingRateOptimalNV", vk::ImageLayout::eShadingRateOptimalNV},
        {"RenderingLocalReadKHR", vk::ImageLayout::eRenderingLocalReadKHR},
        {"VideoEncodeDstKHR", vk::ImageLayout::eVideoEncodeDstKHR},
        {"VideoEncodeSrcKHR", vk::ImageLayout::eVideoEncodeSrcKHR},
        {"VideoEncodeDpbKHR", vk::ImageLayout::eVideoEncodeDpbKHR},
        {"AttachmentFeedbackLoopOptimalEXT", vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT}};

    auto it = imageLayoutMap.find(imageLayout);
    if (it != imageLayoutMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown ImageLayout: " + imageLayout);
}

vk::PipelineStageFlagBits RenderPass::getPipelineStageFlagBitWithJson(const std::string &pipelineStage)
{
    static const std::unordered_map<std::string, vk::PipelineStageFlagBits> pipelineStageMap = {
        {"TopOfPipe", vk::PipelineStageFlagBits::eTopOfPipe},
        {"DrawIndirect", vk::PipelineStageFlagBits::eDrawIndirect},
        {"VertexInput", vk::PipelineStageFlagBits::eVertexInput},
        {"VertexShader", vk::PipelineStageFlagBits::eVertexShader},
        {"TessellationControlShader", vk::PipelineStageFlagBits::eTessellationControlShader},
        {"TessellationEvaluationShader", vk::PipelineStageFlagBits::eTessellationEvaluationShader},
        {"GeometryShader", vk::PipelineStageFlagBits::eGeometryShader},
        {"FragmentShader", vk::PipelineStageFlagBits::eFragmentShader},
        {"EarlyFragmentTests", vk::PipelineStageFlagBits::eEarlyFragmentTests},
        {"LateFragmentTests", vk::PipelineStageFlagBits::eLateFragmentTests},
        {"ColorAttachmentOutput", vk::PipelineStageFlagBits::eColorAttachmentOutput},
        {"ComputeShader", vk::PipelineStageFlagBits::eComputeShader},
        {"Transfer", vk::PipelineStageFlagBits::eTransfer},
        {"BottomOfPipe", vk::PipelineStageFlagBits::eBottomOfPipe},
        {"Host", vk::PipelineStageFlagBits::eHost},
        {"AllGraphics", vk::PipelineStageFlagBits::eAllGraphics},
        {"AllCommands", vk::PipelineStageFlagBits::eAllCommands},
        {"None", vk::PipelineStageFlagBits::eNone},
        {"NoneKHR", vk::PipelineStageFlagBits::eNoneKHR},
        {"TransformFeedbackEXT", vk::PipelineStageFlagBits::eTransformFeedbackEXT},
        {"ConditionalRenderingEXT", vk::PipelineStageFlagBits::eConditionalRenderingEXT},
        {"AccelerationStructureBuildKHR", vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR},
        {"AccelerationStructureBuildNV", vk::PipelineStageFlagBits::eAccelerationStructureBuildNV},
        {"RayTracingShaderKHR", vk::PipelineStageFlagBits::eRayTracingShaderKHR},
        {"RayTracingShaderNV", vk::PipelineStageFlagBits::eRayTracingShaderNV},
        {"FragmentDensityProcessEXT", vk::PipelineStageFlagBits::eFragmentDensityProcessEXT},
        {"FragmentShadingRateAttachmentKHR", vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR},
        {"ShadingRateImageNV", vk::PipelineStageFlagBits::eShadingRateImageNV},
        {"CommandPreprocessNV", vk::PipelineStageFlagBits::eCommandPreprocessNV},
        {"CommandPreprocessEXT", vk::PipelineStageFlagBits::eCommandPreprocessEXT},
        {"TaskShaderEXT", vk::PipelineStageFlagBits::eTaskShaderEXT},
        {"TaskShaderNV", vk::PipelineStageFlagBits::eTaskShaderNV},
        {"MeshShaderEXT", vk::PipelineStageFlagBits::eMeshShaderEXT},
        {"MeshShaderNV", vk::PipelineStageFlagBits::eMeshShaderNV}};

    auto it = pipelineStageMap.find(pipelineStage);
    if (it != pipelineStageMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown PipelineStageFlagBit: " + pipelineStage);
}

vk::AccessFlagBits RenderPass::getAccessFlagBitWithJson(const std::string &access)
{
    static const std::unordered_map<std::string, vk::AccessFlagBits> accessMap = {
        {"IndirectCommandRead", vk::AccessFlagBits::eIndirectCommandRead},
        {"IndexRead", vk::AccessFlagBits::eIndexRead},
        {"VertexAttributeRead", vk::AccessFlagBits::eVertexAttributeRead},
        {"UniformRead", vk::AccessFlagBits::eUniformRead},
        {"InputAttachmentRead", vk::AccessFlagBits::eInputAttachmentRead},
        {"ShaderRead", vk::AccessFlagBits::eShaderRead},
        {"ShaderWrite", vk::AccessFlagBits::eShaderWrite},
        {"ColorAttachmentRead", vk::AccessFlagBits::eColorAttachmentRead},
        {"ColorAttachmentWrite", vk::AccessFlagBits::eColorAttachmentWrite},
        {"DepthStencilAttachmentRead", vk::AccessFlagBits::eDepthStencilAttachmentRead},
        {"DepthStencilAttachmentWrite", vk::AccessFlagBits::eDepthStencilAttachmentWrite},
        {"TransferRead", vk::AccessFlagBits::eTransferRead},
        {"TransferWrite", vk::AccessFlagBits::eTransferWrite},
        {"HostRead", vk::AccessFlagBits::eHostRead},
        {"HostWrite", vk::AccessFlagBits::eHostWrite},
        {"MemoryRead", vk::AccessFlagBits::eMemoryRead},
        {"MemoryWrite", vk::AccessFlagBits::eMemoryWrite},
        {"None", vk::AccessFlagBits::eNone},
        {"NoneKHR", vk::AccessFlagBits::eNoneKHR},
        {"TransformFeedbackWriteEXT", vk::AccessFlagBits::eTransformFeedbackWriteEXT},
        {"TransformFeedbackCounterReadEXT", vk::AccessFlagBits::eTransformFeedbackCounterReadEXT},
        {"TransformFeedbackCounterWriteEXT", vk::AccessFlagBits::eTransformFeedbackCounterWriteEXT},
        {"ConditionalRenderingReadEXT", vk::AccessFlagBits::eConditionalRenderingReadEXT},
        {"ColorAttachmentReadNoncoherentEXT", vk::AccessFlagBits::eColorAttachmentReadNoncoherentEXT},
        {"AccelerationStructureReadKHR", vk::AccessFlagBits::eAccelerationStructureReadKHR},
        {"AccelerationStructureReadNV", vk::AccessFlagBits::eAccelerationStructureReadNV},
        {"AccelerationStructureWriteKHR", vk::AccessFlagBits::eAccelerationStructureWriteKHR},
        {"AccelerationStructureWriteNV", vk::AccessFlagBits::eAccelerationStructureWriteNV},
        {"FragmentDensityMapReadEXT", vk::AccessFlagBits::eFragmentDensityMapReadEXT},
        {"FragmentShadingRateAttachmentReadKHR", vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR},
        {"ShadingRateImageReadNV", vk::AccessFlagBits::eShadingRateImageReadNV},
        {"CommandPreprocessReadNV", vk::AccessFlagBits::eCommandPreprocessReadNV},
        {"CommandPreprocessReadEXT", vk::AccessFlagBits::eCommandPreprocessReadEXT},
        {"CommandPreprocessWriteNV", vk::AccessFlagBits::eCommandPreprocessWriteNV},
        {"CommandPreprocessWriteEXT", vk::AccessFlagBits::eCommandPreprocessWriteEXT}};

    auto it = accessMap.find(access);
    if (it != accessMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown AccessFlagBit: " + access);
}

vk::PipelineStageFlags RenderPass::getPipelineStageFlagsWithJson(const std::vector<std::string> &pipelineStageFlags)
{
    vk::PipelineStageFlags flags;
    for (const std::string &pipelineStageFlagBit : pipelineStageFlags)
        flags |= getPipelineStageFlagBitWithJson(pipelineStageFlagBit);
    return flags;
}

vk::AccessFlags RenderPass::getAccessFlagsWithJson(const std::vector<std::string> &accessFlags)
{
    vk::AccessFlags flags;
    for (const std::string &accessFlagBit : accessFlags)
        flags |= getAccessFlagBitWithJson(accessFlagBit);
    return flags;
}

std::vector<vk::AttachmentDescription> RenderPass::getAttachmentsWithJson(const json &config, const std::string &swapchainName, vk::Format depthFormat)
{
    const json &attachmentsJson = config["attachments"];

    std::vector<vk::AttachmentDescription> attachments(attachmentsJson.size());

    for (uint32_t i = 0; i < attachments.size(); ++i)
    {
        const json &attachmentJson = attachmentsJson[i];
        vk::AttachmentDescription &attachment = attachments[i];

        attachment.setFormat(getFormatWithJson(attachmentJson["format"], swapchainName, depthFormat))
            .setSamples(getSampleCountFlagBitWithJson(attachmentJson["samples"]))
            .setLoadOp(getAttachmentLoadOpWithJson(attachmentJson["loadOp"]))
            .setStoreOp(getAttachmentStoreOpWithJson(attachmentJson["storeOp"]))
            .setStencilLoadOp(getAttachmentLoadOpWithJson(attachmentJson["stencilLoadOp"]))
            .setStencilStoreOp(getAttachmentStoreOpWithJson(attachmentJson["stencilStoreOp"]))
            .setInitialLayout(getImageLayoutWithJson(attachmentJson["initialLayout"]))
            .setFinalLayout(getImageLayoutWithJson(attachmentJson["finalLayout"]));
    }
    return attachments;

}

std::vector<std::vector<vk::AttachmentReference>> RenderPass::getAttachmentRefsWithJson(const json &config)
{
    try
    {
        const json &attachmentRefSsJson = config["attachmentReferences"];

        std::vector<std::vector<vk::AttachmentReference>> attachmentRefSs(attachmentRefSsJson.size());

        for (uint32_t i = 0; i < attachmentRefSs.size(); ++i)
        {
            const json &attachmentRefsJson = attachmentRefSsJson[i];
            std::vector<vk::AttachmentReference> &attachmentRefs = attachmentRefSs[i];
            attachmentRefs.resize(attachmentRefsJson.size());

            for (uint32_t j = 0; j < attachmentRefs.size(); ++j)
            {
                const json &attachmentRefJson = attachmentRefsJson[j];

                vk::AttachmentReference &attachmentRef = attachmentRefs[j];

                attachmentRef.setAttachment(attachmentRefJson["attachment"])
                    .setLayout(getImageLayoutWithJson(attachmentRefJson["layout"]));
            }
        }
        return attachmentRefSs;
    }
    catch (const char *msg)
    {
        throw std::runtime_error(std::string("Config Format Error: ") + msg);
    }
}

std::vector<vk::SubpassDescription> RenderPass::getSubpassesWithJson(const json &config, const std::vector<std::vector<vk::AttachmentReference>> &attachmentRefs)
{
    const json &subpassesJson = config["subpasses"];
    std::vector<vk::SubpassDescription> subpasses(subpassesJson.size());
    for (uint32_t i = 0; i < subpasses.size(); ++i)
    {
        const json &subpassJson = subpassesJson[i];
        vk::SubpassDescription &subpass = subpasses[i];
        auto it = subpassJson.find("colorAttachments");
        if (it != subpassJson.end())
            subpass.setColorAttachments(attachmentRefs[it.value()]);
        
        it = subpassJson.find("inputAttachments");
        if (it != subpassJson.end())
            subpass.setInputAttachments(attachmentRefs[it.value()]);
        
        it = subpassJson.find("depthStencilAttachment");
        if (it != subpassJson.end())
            subpass.setPDepthStencilAttachment(&attachmentRefs[it.value()][0]);
    }
    return subpasses; 
}

std::vector<vk::SubpassDependency> RenderPass::getSubpassDependenciesWithJson(const json &config)
{
    const json &subpassDependenciesJson = config["subpassDependencies"];
    std::vector<vk::SubpassDependency> subpassDependencies(subpassDependenciesJson.size());

    for (uint32_t i = 0; i < subpassDependencies.size(); ++i)
    {
        const json &subpassDependencyJson = subpassDependenciesJson[i];
        vk::SubpassDependency &subpassDependency = subpassDependencies[i];
        subpassDependency.setSrcSubpass(subpassDependencyJson["srcSubpass"])
            .setDstSubpass(subpassDependencyJson["dstSubpass"])
            .setSrcStageMask(getPipelineStageFlagsWithJson(subpassDependencyJson["srcStageMask"]))
            .setSrcAccessMask(getAccessFlagsWithJson(subpassDependencyJson["srcAccessMask"]))
            .setDstStageMask(getPipelineStageFlagsWithJson(subpassDependencyJson["dstStageMask"]))
            .setDstAccessMask(getAccessFlagsWithJson(subpassDependencyJson["dstAccessMask"]));
    }

    return subpassDependencies;
}

vk::RenderPassCreateInfo RenderPass::getRenderPassCreateInfo(const std::vector<vk::AttachmentDescription> &attachments, const std::vector<vk::SubpassDescription> &subpasses, const std::vector<vk::SubpassDependency> &subpassDependencies)
{
    return vk::RenderPassCreateInfo().setAttachments(attachments).setSubpasses(subpasses).setDependencies(subpassDependencies);
}

} // namespace Vkbase