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
    friend class ResourceManager;

  public:
    void draw();
    void sizeChanged();
    void setCommandRecordFunc(const std::function<void(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)> &func);
    void setRenderPassCreateFunc(const std::function<void()> &func);
    static uint32_t maxFlightCount();

  private:
    RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::string &swapchainName,
                    const std::string &commandPoolName);
    ~RenderDelegator() override;
    void init();
    void createSyncObjects();
    void recreateSwapchain();
    const Device &_device;
    inline static uint32_t _maxFlightCount = MAX_FLIGHT_COUNT;
    Swapchain *_pSwapchain;
    const CommandPool &_commandPool;
    std::vector<vk::CommandBuffer> _commandBuffers;
    std::vector<vk::Semaphore> _imageAvailableSemaphores;
    std::vector<vk::Semaphore> _renderFinishSemaphores;
    std::vector<vk::Fence> _inFlightFences;
    uint32_t _currentFrame = 0;
    bool _isSizeChanged = false;
    std::function<void(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)> _commandRecordFunc;
    std::function<void()> _renderPassCreateFunc;
};
} // namespace Vkbase