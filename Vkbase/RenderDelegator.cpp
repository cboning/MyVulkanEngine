#include "RenderDelegator.h"
#include "CommandPool.h"
#include "Device.h"
#include "Swapchain.h"
#include <iostream>
#include <limits>

namespace Vkbase
{
RenderDelegator::RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::string &swapchainName,
                                 const std::string &commandPoolName)
    : ResourceBase(Vkbase::ResourceType::RenderDelegator, resourceName),
      _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName)))),
      _commandPool(*dynamic_cast<const CommandPool *>(connectTo(resourceManager().resource(Vkbase::ResourceType::CommandPool, commandPoolName))))
{
    _pSwapchain = dynamic_cast<Swapchain *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Swapchain, swapchainName)));
    init();
}

RenderDelegator::~RenderDelegator()
{
    _device.device().waitIdle();
    _commandPool.freeCommandBuffers(_commandBuffers);

    for (const auto &semaphore : _renderFinishSemaphores)
        _device.device().destroySemaphore(semaphore);
    _renderFinishSemaphores.clear();

    for (const auto &semaphore : _imageAvailableSemaphores)
        _device.device().destroySemaphore(semaphore);
    _imageAvailableSemaphores.clear();

    for (const vk::Fence &fence : _inFlightFences)
        _device.device().destroyFence(fence);
    _inFlightFences.clear();
}

void RenderDelegator::init()
{
    _commandBuffers = _commandPool.allocateFlightCommandBuffers(_maxFlightCount);
    createSyncObjects();
}

void RenderDelegator::recreateSwapchain()
{
    _device.device().waitIdle();
    auto renderPassCreateFunc = _renderPassCreateFunc;
    auto pSwapchain = _pSwapchain;
    pSwapchain->recreate();

    if (renderPassCreateFunc)
        renderPassCreateFunc();
}

void RenderDelegator::createSyncObjects()
{
    _imageAvailableSemaphores.resize(_maxFlightCount);
    _renderFinishSemaphores.resize(_maxFlightCount);
    _inFlightFences.resize(_maxFlightCount);
    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    for (uint32_t i = 0; i < _maxFlightCount; ++i)
    {
        _imageAvailableSemaphores[i] = _device.device().createSemaphore(semaphoreCreateInfo);
        _renderFinishSemaphores[i] = _device.device().createSemaphore(semaphoreCreateInfo);

        _inFlightFences[i] = _device.device().createFence(fenceCreateInfo);
    }
}

void RenderDelegator::draw()
{
    if (!_commandRecordFunc)
        return;
    (void)_device.device().waitForFences(_inFlightFences[_currentFrame], vk::True, UINT64_MAX);
    vk::Semaphore acquireSemaphore = _imageAvailableSemaphores[_currentFrame];
    vk::ResultValue<uint32_t> uintResult =
        _device.device().acquireNextImageKHR(_pSwapchain->swapchain(), std::numeric_limits<uint64_t>::max(), acquireSemaphore, nullptr);
    uint32_t imageIndex = uintResult.value;
    if (uintResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        // 处理重建
        recreateSwapchain();
        return;
    }
    else if (uintResult.result != vk::Result::eSuccess && uintResult.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    _device.device().resetFences(_inFlightFences[_currentFrame]);
    vk::CommandBuffer &commandBuffer = _commandBuffers[_currentFrame];
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse).setPInheritanceInfo(nullptr);
    commandBuffer.reset();
    commandBuffer.begin(beginInfo);
    // 这里只传第一个 swapchain 的 imageIndex，实际可根据需求扩展
    _commandRecordFunc(commandBuffer, imageIndex, _currentFrame);
    commandBuffer.end();
    // signalSemaphores 取所有 renderFinishSemaphores
    vk::Semaphore signalSemaphores = _renderFinishSemaphores[_currentFrame];
    std::vector<vk::PipelineStageFlags> stageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer).setSignalSemaphores(signalSemaphores).setWaitDstStageMask(stageFlags).setWaitSemaphores(acquireSemaphore);
    try
    {
        _device.graphicsQueue().submit(submitInfo, _inFlightFences[_currentFrame]);
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

void RenderDelegator::setCommandRecordFunc(const std::function<void(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)> &func)
{
    _commandRecordFunc = func;
}

void RenderDelegator::setRenderPassCreateFunc(const std::function<void()> &func) { _renderPassCreateFunc = func; }
} // namespace Vkbase