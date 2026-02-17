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
      _device(connectTo(resourceManager().resource(Vkbase::VkResourceType::Device, deviceName))),
      _commandPool(connectTo(resourceManager().resource(Vkbase::VkResourceType::CommandPool, commandPoolName)))
{
    _swapchain = connectTo(resourceManager().resource(Vkbase::VkResourceType::Swapchain, swapchainName));
    init();
}

RenderDelegator::~RenderDelegator()
{
    if (auto p = _device.lock<Device>())
    {
        p->device().waitIdle();

        for (const auto &semaphore : _renderFinishSemaphores)
            p->device().destroySemaphore(semaphore);
        _renderFinishSemaphores.clear();

        for (const auto &semaphore : _imageAvailableSemaphores)
            p->device().destroySemaphore(semaphore);
        _imageAvailableSemaphores.clear();
    }

    if (auto p = _commandPool.lock<CommandPool>())
        p->freeCommandBuffers(_commandBuffers);
}

void RenderDelegator::init()
{
    if (auto p = _commandPool.lock<CommandPool>())
        _commandBuffers = p->allocateFlightCommandBuffers(_maxFlightCount);
    createSyncObjects();
}

void RenderDelegator::recreateSwapchain()
{
    if (auto p = _device.lock<Device>())
        p->device().waitIdle();
    _currentFrame = 0;
    for (auto pCommandBuffer : _commandBuffers)
        if (auto p = pCommandBuffer.lock<CommandBuffer>())
            p->reset();
    auto renderPassCreateFunc = _renderPassCreateFunc;

    _swapchainRecreatePrefunc();
    disconnectTo(_swapchain);
    if (auto p = _swapchain.lock<Swapchain>())
        _swapchain = connectTo(p->recreate());

    if (renderPassCreateFunc)
        renderPassCreateFunc();
}

void RenderDelegator::createSyncObjects()
{
    _imageAvailableSemaphores.resize(_maxFlightCount);
    _renderFinishSemaphores.resize(_maxFlightCount);

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    if (auto p = _device.lock<Device>())
    {
        const auto &device = p->device();
        for (uint32_t i = 0; i < _maxFlightCount; ++i)
        {
            _imageAvailableSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
            _renderFinishSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
        }
    }
}

void RenderDelegator::draw()
{
    if (!_commandRecordFunc)
        return;
    auto commandBuffer = _commandBuffers[_currentFrame];
    if (auto p = commandBuffer.lock<CommandBuffer>())
        p->waitForFence();
    _updateFunc(_currentFrame);

    vk::Semaphore acquireSemaphore = _imageAvailableSemaphores[_currentFrame];

    uint32_t imageIndex;
    if (auto p = _device.lock<Device>())
    {
        if (auto p1 = _swapchain.lock<Swapchain>())
        {
            vk::ResultValue<uint32_t> uintResult =
                p->device().acquireNextImageKHR(p1->swapchain(), std::numeric_limits<uint64_t>::max(), acquireSemaphore, nullptr);
            if (uintResult.result == vk::Result::eErrorOutOfDateKHR)
            {
                recreateSwapchain();
                return;
            }
            else if (uintResult.result != vk::Result::eSuccess && uintResult.result != vk::Result::eSuboptimalKHR)
                throw std::runtime_error("Failed to acquire swap chain image!");

            imageIndex = uintResult.value;
        }
        else
            throw std::runtime_error("Swapchain already destroyed.");
    }
    else
        throw std::runtime_error("Device already destroyed.");

    if (auto p = commandBuffer.lock<CommandBuffer>())
    {
        p->reset();
        p->begin();
        _commandRecordFunc(commandBuffer, imageIndex, _currentFrame);
        p->end();
    }

    vk::Semaphore signalSemaphores = _renderFinishSemaphores[_currentFrame];
    std::vector<vk::PipelineStageFlags> stageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    try
    {
        if (auto p = commandBuffer.lock<CommandBuffer>())
            p->submit({acquireSemaphore}, stageFlags, {signalSemaphores});
        else
            throw std::runtime_error("CommandBuffer already destroyed.");
    }
    catch (const vk::SystemError &err)
    {
        std::cerr << "Vulkan error: " << err.what() << std::endl;
    }

    std::vector<vk::SwapchainKHR> vkSwapchains;
    if (auto p = _swapchain.lock<Swapchain>())
        vkSwapchains.push_back(p->swapchain());

    vk::PresentInfoKHR presentInfo;
    presentInfo.setImageIndices(imageIndex).setSwapchains(vkSwapchains).setWaitSemaphores(signalSemaphores);
    vk::Result presentResult;
    if (auto p = _device.lock<Device>())
        presentResult = p->presentQueue().presentKHR(presentInfo);
    else
        throw std::runtime_error("Failed to present swap chain image! Device was destroyed.");

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

void RenderDelegator::setCommandRecordFunc(
    const std::function<void(const VkResourceManagerHolder::WeakReference &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)> &func)
{
    _commandRecordFunc = func;
}

void RenderDelegator::setUpdateFunc(const std::function<void(uint32_t currentFrame)> &func) { _updateFunc = func; }

void RenderDelegator::setRenderPassCreateFunc(const std::function<void()> &func) { _renderPassCreateFunc = func; }

void RenderDelegator::setSwapchainRecreatePrefunc(const std::function<void()> &func) { _swapchainRecreatePrefunc = func; }
} // namespace Vkbase