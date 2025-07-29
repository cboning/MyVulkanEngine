#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
    class Device;
    class Buffer;
    class Swapchain;
    class Image : public ResourceBase
    {
    public:
        Image(const std::string &resourceName, const std::string &deviceName, const std::string &filename, vk::Format format, vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage);
        Image(const std::string &resourceName, const std::string &deviceName, uint32_t width, uint32_t height, uint32_t depth, vk::Format format, vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage, void *pData);
        Image(const std::string &resourceName, const std::string &deviceName, uint32_t width, uint32_t height, uint32_t depth, vk::Format format, vk::ImageType type, vk::ImageViewType viewType, vk::ImageUsageFlags usage);
        ~Image() override;
        
        const vk::Image &image() const;
        const vk::ImageView &view() const;
        vk::Format format() const;
        static const std::vector<std::string> getImagesWithSwapchain(const Swapchain &swapchain);
        void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
        
    private:
        Image(const Swapchain &swapchain, uint32_t index);
        const Device *_pDevice;
        vk::Image _image;
        vk::DeviceMemory _memory;
        vk::ImageView _view;
        const vk::Format _format;
        const vk::ImageType _type;
        const vk::ImageViewType _viewType;

        void loadImage(std::string fileName, vk::ImageUsageFlags usage);
        void createImageView();
        void createImage(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
        void createImageWithData(uint32_t width, uint32_t height, uint32_t depth, vk::ImageUsageFlags usage, void *pData);
        void copyBufferDataToImage(const Buffer &buffer, uint32_t width, uint32_t height, uint32_t depth);
        uint32_t findMemoryType(uint32_t filterType, vk::MemoryPropertyFlags properties);
        bool isDepthImage();
        static size_t getPixelSize(vk::Format format);
        
    };
}