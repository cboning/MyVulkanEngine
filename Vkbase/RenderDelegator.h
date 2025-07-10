#pragma once
#include "ResourceBase.h"
#include <functional>
#define MAX_FLIGHT_COUNT 3

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
        void setCommandRecordFunc(const std::function<void(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)>& func);
        void setRenderPassCreateFunc(const std::function<void()>& func);
        static uint32_t maxFlightCount();
    private:
        void init();
        void createSyncObjects();
        void recreateSwapchain(int32_t swapchainIndex);
        const Device &_device;
        inline static uint32_t _maxFlightCount = MAX_FLIGHT_COUNT;
        std::vector<Swapchain *> _pSwapchains;
        const CommandPool &_commandPool;
        std::vector<vk::CommandBuffer> _commandBuffers;
        std::vector<std::vector<vk::Semaphore>> _imageAvailableSemaphores;
        std::vector<std::vector<vk::Semaphore>> _renderFinishSemaphores;
        std::vector<vk::Fence> _inFlightFences;
        uint32_t _currentFrame = 0;
        bool _isSizeChanged = false;
        std::function<void(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)> _commandRecordFunc;
        std::function<void()> _renderPassCreateFunc;
    };
}