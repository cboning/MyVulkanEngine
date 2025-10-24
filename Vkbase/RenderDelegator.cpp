#include "RenderDelegator.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"
#include "Swapchain.h"
#include <iostream>
#include <limits>

namespace Vkbase
{
RenderDelegator::RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::string &swapchainName,
                                 const std::string &commandPoolName)
    : VkResourceBase(Vkbase::VkResourceType::RenderDelegator, resourceName),
      _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)))),
      _commandPool(*dynamic_cast<const CommandPool *>(connectTo(resourceManager().resource(Vkbase::VkResourceType::CommandPool, commandPoolName))))
{
    _pSwapchain = dynamic_cast<Swapchain *>(resourceManager().resource(Vkbase::VkResourceType::Swapchain, swapchainName));
    init();
}

RenderDelegator::~RenderDelegator()
{
    _device.device().waitIdle();
    _commandPool.freeCommandBuffers(_pCommandBuffers);

    for (const auto &semaphore : _renderFinishSemaphores)
        _device.device().destroySemaphore(semaphore);
    _renderFinishSemaphores.clear();

    for (const auto &semaphore : _imageAvailableSemaphores)
        _device.device().destroySemaphore(semaphore);
    _imageAvailableSemaphores.clear();
}

void RenderDelegator::init()
{
    _pCommandBuffers = _commandPool.allocateFlightCommandBuffers(_maxFlightCount);
    createSyncObjects();
}

void RenderDelegator::recreateSwapchain()
{
    _device.device().waitIdle();
    for (auto pCommandBuffer : _pCommandBuffers)
        pCommandBuffer->reset();
    auto renderPassCreateFunc = _renderPassCreateFunc;
    _pSwapchain = _pSwapchain->recreate();

    if (renderPassCreateFunc)
        renderPassCreateFunc();
    _currentFrame = 0;
}

void RenderDelegator::createSyncObjects()
{
    _imageAvailableSemaphores.resize(_maxFlightCount);
    _renderFinishSemaphores.resize(_maxFlightCount);

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    const auto &device = _device.device();

    for (uint32_t i = 0; i < _maxFlightCount; ++i)
    {
        _imageAvailableSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
        _renderFinishSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
    }
}

void RenderDelegator::draw()
{
    if (!_commandRecordFunc)
        return;
    CommandBuffer *pCommandBuffer = _pCommandBuffers[_currentFrame];
    pCommandBuffer->waitForFence();
    _updateFunc(_currentFrame);

    vk::Semaphore acquireSemaphore = _imageAvailableSemaphores[_currentFrame];

    vk::ResultValue<uint32_t> uintResult =
        _device.device().acquireNextImageKHR(_pSwapchain->swapchain(), std::numeric_limits<uint64_t>::max(), acquireSemaphore, nullptr);
    if (uintResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (uintResult.result != vk::Result::eSuccess && uintResult.result != vk::Result::eSuboptimalKHR)
        throw std::runtime_error("Failed to acquire swap chain image!");

    uint32_t imageIndex = uintResult.value;

    pCommandBuffer->reset();
    pCommandBuffer->begin();
    _commandRecordFunc(pCommandBuffer, imageIndex, _currentFrame);
    pCommandBuffer->end();

    vk::Semaphore signalSemaphores = _renderFinishSemaphores[_currentFrame];
    std::vector<vk::PipelineStageFlags> stageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    try
    {
        pCommandBuffer->submit({acquireSemaphore}, stageFlags, {signalSemaphores});
    }
    catch (const vk::SystemError &err)
    {
        std::cerr << "Vulkan error: " << err.what() << std::endl;
    }

    std::vector<vk::SwapchainKHR> vkSwapchains;
    vkSwapchains.push_back(_pSwapchain->swapchain());

    vk::PresentInfoKHR presentInfo;
    presentInfo.setImageIndices(imageIndex).setSwapchains(vkSwapchains).setWaitSemaphores(signalSemaphores);
    vk::Result presentResult = _device.presentQueue().presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || _isSizeChanged)
    {
        _isSizeChanged = false;
        // 处理重建
        recreateSwapchain();
        return;
    }
    else if (presentResult != vk::Result::eSuccess)
        throw std::runtime_error("Failed to present swap chain image!");
    _currentFrame = (_currentFrame + 1) % _maxFlightCount;
}

void RenderDelegator::sizeChanged() { _isSizeChanged = true; }

uint32_t RenderDelegator::maxFlightCount() { return _maxFlightCount; }

void RenderDelegator::setCommandRecordFunc(const std::function<void(CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)> &func)
{
    _commandRecordFunc = func;
}

void RenderDelegator::setUpdateFunc(const std::function<void(uint32_t currentFrame)> &func) { _updateFunc = func; }

void RenderDelegator::setRenderPassCreateFunc(const std::function<void()> &func) { _renderPassCreateFunc = func; }
} // namespace Vkbase