#pragma once
#include "VkResourceBase.h"
#include <functional>
#define MAX_FLIGHT_COUNT 3

namespace Vkbase
{
class Device;
class Swapchain;
class CommandPool;
class CommandBuffer;
class RenderDelegator : public VkResourceBase
{
    friend class VkResourceManager;

public:
    void draw();
    void sizeChanged();
    void setCommandRecordFunc(const std::function<void(CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)> &func);
    void setUpdateFunc(const std::function<void(uint32_t currentFrame)> &func);
    void setRenderPassCreateFunc(const std::function<void()> &func);
    void setSwapchainRecreatePrefunc(const std::function<void()> &func);
    static uint32_t maxFlightCount();
    void recreateSwapchain();

private:
    RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::string &swapchainName, const std::string &commandPoolName);
    ~RenderDelegator() override;
    void init();
    void createSyncObjects();
    const Device &_device;
    inline static uint32_t _maxFlightCount = MAX_FLIGHT_COUNT;
    Swapchain *_pSwapchain = nullptr;
    const CommandPool &_commandPool;
    std::vector<CommandBuffer *> _pCommandBuffers;
    std::vector<vk::Semaphore> _imageAvailableSemaphores;
    std::vector<vk::Semaphore> _renderFinishSemaphores;
    uint32_t _currentFrame = 0;
    bool _isSizeChanged = false;
    std::function<void(CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)> _commandRecordFunc;
    std::function<void(uint32_t currentFrame)> _updateFunc;
    std::function<void()> _renderPassCreateFunc;
    std::function<void()> _swapchainRecreatePrefunc;
};
} // namespace Vkbase