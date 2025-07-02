#pragma once
#include "ResourceBase.h"

namespace Vkbase
{
    class Device;
    class Swapchain;
    class CommandPool;
    class RenderDelegator : public ResourceBase
    {
    public:
        RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName, const std::string &swapchainName, const std::string &commandPoolName);
        ~RenderDelegator();
        void sizeChanged();
        void draw();
        void setCommandRecordFunc(void (* recordFunc)(ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex));
    private:
        const Device &_device;
        const Swapchain &_swapchain;
        const CommandPool &_commandPool;
        uint32_t _currentFrame = 0;
        bool _isSizeChanged = false;
        std::vector<vk::Semaphore> _imageAvailableSemaphores;
        std::vector<vk::Semaphore> _renderFinishSemaphores;
        std::vector<vk::Fence> _inFlightFences;
        std::vector<vk::CommandBuffer> _commandBuffers;

        void (* _commandRecordFunc)(ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex) = nullptr;

        void init();
        void createSyncObjects();
    };
}