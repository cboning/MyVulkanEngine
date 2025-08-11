#include "Image.h"
#include "Device.h"
#include "CommandPool.h"
#include "Buffer.h"
#include "Swapchain.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

namespace Vkbase
{
    Image::Image(const std::string &resourceName, const std::string &deviceName, const std::string &filename, vk::Format format, vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage)
        : ResourceBase(Vkbase::ResourceType::Image, resourceName), _pDevice(dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName)))), _format(format), _type(type), _viewType(viewType)
    {
        loadImage(filename, usage);
    }

    Image::Image(const std::string &resourceName, const std::string &deviceName, uint32_t width, uint32_t height, uint32_t depth, vk::Format format, vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage, const void *pData)
        : ResourceBase(Vkbase::ResourceType::Image, resourceName), _pDevice(dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName)))), _format(format), _type(type), _viewType(viewType)
    {
        createImageWithData(width, height, depth, usage, pData);
    }

    Image::Image(const std::string &resourceName, const std::string &deviceName, uint32_t width, uint32_t height, uint32_t depth, vk::Format format, vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage)
        : ResourceBase(Vkbase::ResourceType::Image, resourceName), _pDevice(dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName)))), _format(format), _type(type), _viewType(viewType)
    {
        createImage(width, height, depth, usage);
        if (usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
            transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        else if (usage & vk::ImageUsageFlagBits::eColorAttachment)
            transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
        else if (usage & vk::ImageUsageFlagBits::eStorage)
            transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        createImageView();
    }

    Image::Image(const Swapchain &swapchain, uint32_t index)
        : ResourceBase(Vkbase::ResourceType::Image, swapchain.name() + "_" + std::to_string(index)),
          _pDevice(nullptr),
          _image(swapchain.images()[index]),
          _view(swapchain.imageViews()[index]),
          _format(swapchain.format()),
          _type(vk::ImageType::e2D),
          _viewType(vk::ImageViewType::e2D)
    {
        connectTo(&swapchain);
    }

    Image::~Image()
    {
        if (!_pDevice)
            return ;
        _pDevice->device().destroy(_view);
        _pDevice->device().destroy(_image);
        _pDevice->device().freeMemory(_memory);
    }

    void Image::createImageWithData(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage, const void *pData)
    {
        vk::DeviceSize imageSize = width * height * depth * getPixelSize(_format);
        Buffer *buffer = new Buffer("temp", _pDevice->name(), imageSize, vk::BufferUsageFlagBits::eTransferSrc, nullptr);
        buffer->updateBufferData(pData);

        createImage(width, height, depth, vk::ImageUsageFlagBits::eTransferDst | usage);
        transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        copyBufferDataToImage(*buffer, width, height, depth);
        transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        buffer->destroy();
        createImageView();
    }


    void Image::loadImage(std::string fileName, vk::ImageUsageFlags usage)
    {
        int width, height, channels;
        std::replace(fileName.begin(), fileName.end(), '\\', '/');
        stbi_uc *pData = stbi_load(fileName.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pData)
        {
#ifdef DEBUG
            std::cerr << "[Warning] Failed to load image: " << fileName.c_str() << std::endl;
#endif
            uint32_t empty_color = 0xFFFF00FF;
            createImageWithData(1, 1, 1, usage, &empty_color);
            return;
        }
        createImageWithData(width, height, 1, usage, pData);

        stbi_image_free(pData);
    }

    void Image::copyBufferDataToImage(const Buffer &buffer, uint32_t width, uint32_t height, uint32_t depth)
    {
        const CommandPool &commandPool = CommandPool::getCommandPool(_pDevice->name(), Vkbase::CommandPoolQueueType::Graphics);
        vk::CommandBuffer commandBuffer = commandPool.allocateOnceCommandBuffer();
        commandBuffer.copyBufferToImage(buffer.buffer(), _image, vk::ImageLayout::eTransferDstOptimal, vk::BufferImageCopy().setBufferRowLength(width).setBufferImageHeight(height).setBufferOffset(0).setImageExtent({width, height, depth}).setImageOffset({0, 0, 0}).setImageSubresource(vk::ImageSubresourceLayers().setAspectMask(vk::ImageAspectFlagBits::eColor).setBaseArrayLayer(0).setLayerCount(1).setMipLevel(0)));
        commandPool.endOnceCommandBuffer(commandBuffer);
    }

    void Image::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        vk::ImageSubresourceRange subresourceRange;
        subresourceRange.setAspectMask(isDepthImage() ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor)
            .setBaseArrayLayer(0)
            .setLayerCount(1)
            .setBaseMipLevel(0)
            .setLevelCount(1);

        vk::ImageMemoryBarrier barrier;
        barrier.setImage(_image)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setSubresourceRange(subresourceRange);
        vk::PipelineStageFlags srcStage, dstStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eTransfer;
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            srcStage = vk::PipelineStageFlagBits::eTransfer;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        }
        else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
        {
            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
        } 
        else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
        {
            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
        }
        else if (oldLayout == vk::ImageLayout::eGeneral && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            srcStage = vk::PipelineStageFlagBits::eComputeShader;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        }
        else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral)
        {
            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eComputeShader;
            const CommandPool &commandPool = CommandPool::getCommandPool(_pDevice->name(), Vkbase::CommandPoolQueueType::Compute);
            const vk::CommandBuffer commandBuffer = commandPool.allocateOnceCommandBuffer();
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eShaderWrite);

            commandBuffer.pipelineBarrier(srcStage, dstStage, {}, {}, nullptr, barrier);

            commandPool.endOnceCommandBuffer(commandBuffer);
            return ;
        }
        else
        {
            throw std::runtime_error("Unsupported layout transition!");
        }

        const CommandPool &commandPool = CommandPool::getCommandPool(_pDevice->name(), Vkbase::CommandPoolQueueType::Graphics);
        const vk::CommandBuffer commandBuffer = commandPool.allocateOnceCommandBuffer();

        commandBuffer.pipelineBarrier(srcStage, dstStage, {}, {}, nullptr, barrier);

        commandPool.endOnceCommandBuffer(commandBuffer);
    }

    void Image::createImage(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers)
    {
        vk::Extent3D extent;
        extent.setWidth(width)
            .setHeight(height)
            .setDepth(depth);
        vk::ImageCreateInfo createInfo;
        createInfo.setFormat(_format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setImageType(_type)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(usage)
            .setMipLevels(mipLevels)
            .setExtent(extent)
            .setArrayLayers(arrayLayers);
        _image = _pDevice->device().createImage(createInfo);

        vk::MemoryRequirements requirements = _pDevice->device().getImageMemoryRequirements(_image);
        vk::MemoryAllocateInfo allocateInfo;
        allocateInfo.setAllocationSize(requirements.size)
            .setMemoryTypeIndex(findMemoryType(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
        _memory = _pDevice->device().allocateMemory(allocateInfo);

        _pDevice->device().bindImageMemory(_image, _memory, 0);
    }

    uint32_t Image::findMemoryType(uint32_t filterType, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        physicalDeviceMemoryProperties = _pDevice->physicalDevice().getMemoryProperties();
        for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
        {
            if (filterType & (1 << i) && (properties & physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags) == properties)
                return i;
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    void Image::createImageView()
    {
        vk::ImageSubresourceRange subresource;
        vk::ImageViewCreateInfo createInfo;
        subresource.setAspectMask(isDepthImage() ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor)
            .setBaseArrayLayer(0)
            .setLayerCount(1)
            .setBaseMipLevel(0)
            .setLevelCount(1);
        
        createInfo.setViewType(_viewType)
            .setImage(_image)
            .setFormat(_format)
            .setSubresourceRange(subresource);

        _view = _pDevice->device().createImageView(createInfo);
    }
    
    bool Image::isDepthImage()
    {
        return vk::Format::eD32Sfloat == _format || vk::Format::eD32SfloatS8Uint == _format || vk::Format::eD24UnormS8Uint == _format;
    }

    size_t Image::getPixelSize(vk::Format format) 
    {
        switch (format) {
            case vk::Format::eR8Unorm:
            case vk::Format::eR8Snorm:
            case vk::Format::eR8Uint:
            case vk::Format::eR8Sint:
                return 1;

            case vk::Format::eR8G8Unorm:
            case vk::Format::eR8G8Snorm:
            case vk::Format::eR16Uint:
                return 2;

            case vk::Format::eR8G8B8A8Unorm:
            case vk::Format::eB8G8R8A8Unorm:
            case vk::Format::eR8G8B8A8Srgb:
            case vk::Format::eR32Uint:
            case vk::Format::eD32Sfloat:
                return 4;

            case vk::Format::eR32G32Sfloat:
                return 8;

            case vk::Format::eR32G32B32A32Sfloat:
                return 16;

            case vk::Format::eBc1RgbUnormBlock:
                return 8;

            default:
                throw std::runtime_error("Unsupported format for pixel size calculation");
        }
    }

    const vk::Image &Image::image() const
    {
        return _image;
    }

    const vk::ImageView &Image::view() const
    {
        return _view;
    }

    vk::Format Image::format() const
    {
        return _format;
    }

    const std::vector<std::string> Image::getImagesWithSwapchain(const Swapchain &swapchain)
    {
        std::vector<std::string> imageNames;
        for (uint32_t i = 0; i < swapchain.images().size(); ++i)
        {
            Image *pImage = new Image(swapchain, i);
            imageNames.push_back(pImage->name());
            pImage->setLock();
        }
        return imageNames;
    }
}