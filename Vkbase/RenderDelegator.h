#pragma once
#include "ResourceBase.h"
#include <functional>

namespace Vkbase
{
    class Device;
    class Swapchain;
    class CommandPool;
    class RenderDelegator : public ResourceBase
    {
    public:
        RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::vector<std::string> &swapchainNames, const std::string &commandPoolName);
        ~RenderDelegator();
        void draw();
        void sizeChanged();
        void setCommandRecordFunc(void (* recordFunc)(ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex));
        void setRenderPassCreateFunc(const std::function<void()>& func);
    private:
        void init();
        void createSyncObjects();
        void recreateSwapchain(int32_t swapchainIndex);
        const Device &_device;
        std::vector<Swapchain *> _pSwapchains;
        const CommandPool &_commandPool;
        std::vector<vk::CommandBuffer> _commandBuffers;
        std::vector<std::vector<vk::Semaphore>> _imageAvailableSemaphores;
        std::vector<std::vector<vk::Semaphore>> _renderFinishSemaphores;
        std::vector<vk::Fence> _inFlightFences;
        uint32_t _currentFrame = 0;
        bool _isSizeChanged = false;
        void (* _commandRecordFunc)(ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex) = nullptr;
        std::function<void()> _renderPassCreateFunc;
    };
}