#pragma once
#ifndef DEBUG
#define DEBUG 0 // 默认值
#endif
#include "VkGpuResourceBase.h"
#include <json.hpp>
#include "MemoryAllocator.h"

using json = nlohmann::json;

namespace Vkbase
{
class Swapchain;
class Image : public VkGpuResourceBase
{
    friend class VkResourceManager;
    friend class DescriptorSets;
    friend class Framebuffer;

public:
    vk::Format format() const;
    static const std::vector<std::string> getImagesWithSwapchain(Swapchain &swapchain);
    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

private:
    Image(const std::string &resourceName, const std::string &deviceName, const std::string &filename, vk::Format format, vk::ImageType type,
          vk::ImageViewType viewType, vk::ImageUsageFlags usage);
    Image(const std::string &resourceName, const std::string &deviceName, uint32_t width, uint32_t height, uint32_t depth, vk::Format format,
          vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage, const void *pData);
    Image(const std::string &resourceName, const std::string &deviceName, uint32_t width, uint32_t height, uint32_t depth, vk::Format format,
          vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage);
    Image(const std::string &resourceName, const std::string &deviceName, json config, const void *pData = nullptr, const std::string &swapchainName = "",
          vk::Format depthFormat = vk::Format());
    Image(Swapchain &swapchain, uint32_t index);
    ~Image() override;
    vk::Image _image;
    MemoryAllocator::Allocation _memory;
    vk::ImageView _view;
    const vk::Format _format;
    const vk::ImageType _type;
    const vk::ImageViewType _viewType;
    const bool _destroyDelegate = true;

    void loadImage(std::string fileName, vk::ImageUsageFlags usage);
    void createImageView();
    void createImage(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
    void createImageWithNoData(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage);
    void createImageWithData(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage, const void *pData);
    void copyBufferDataToImage(const VkResourceManagerHolder::WeakReference &buffer, uint32_t width, uint32_t height, uint32_t depth);
    uint32_t findMemoryType(uint32_t filterType, vk::MemoryPropertyFlags properties);
    bool isDepthImage();
    static size_t getPixelSize(vk::Format format);
    const vk::Image &image() const;
    const vk::ImageView &view() const;
};
} // namespace Vkbase