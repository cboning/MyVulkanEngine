#include "RenderDelegator.h"
#include "CommandPool.h"
#include "Device.h"
#include "Swapchain.h"
#include <iostream>
#include <limits>

namespace Vkbase
{
RenderDelegator::RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::vector<std::string> &swapchainNames,
                                 const std::string &commandPoolName)
    : ResourceBase(Vkbase::ResourceType::RenderDelegator, resourceName),
      _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName)))),
      _commandPool(*dynamic_cast<const CommandPool *>(connectTo(resourceManager().resource(Vkbase::ResourceType::CommandPool, commandPoolName))))
{
    for (const auto &swapchainName : swapchainNames)
        _pSwapchains.emplace_back(dynamic_cast<Swapchain *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Swapchain, swapchainName))));
    init();
}

RenderDelegator::~RenderDelegator()
{
    _device.device().waitIdle();
    _commandPool.freeCommandBuffers(_commandBuffers);

    for (const auto &semaphoreVec : _renderFinishSemaphores)
        for (const vk::Semaphore &semaphore : semaphoreVec)
            _device.device().destroySemaphore(semaphore);
    _renderFinishSemaphores.clear();

    for (const auto &semaphoreVec : _imageAvailableSemaphores)
        for (const vk::Semaphore &semaphore : semaphoreVec)
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

void RenderDelegator::recreateSwapchain(int32_t swapchainIndex)
{
    _device.device().waitIdle();
    auto renderPassCreateFunc = _renderPassCreateFunc;
    auto pSwapchains = _pSwapchains;
    if (swapchainIndex == -1)
    {
        for (uint32_t i = 0; i < pSwapchains.size(); ++i)
            pSwapchains[i]->recreate();
    }
    else
        pSwapchains[swapchainIndex]->recreate();

    if (renderPassCreateFunc)
        renderPassCreateFunc();
}

void RenderDelegator::createSyncObjects()
{
    // 为每个 swapchain/帧分配独立信号量
    size_t swapchainCount = _pSwapchains.size();
    _imageAvailableSemaphores.resize(swapchainCount);
    _renderFinishSemaphores.resize(swapchainCount);
    for (size_t i = 0; i < swapchainCount; ++i)
    {
        _imageAvailableSemaphores[i].resize(_maxFlightCount);
        _renderFinishSemaphores[i].resize(_maxFlightCount);
    }
    _inFlightFences.resize(_maxFlightCount);
    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    for (uint32_t i = 0; i < _maxFlightCount; ++i)
    {
        for (size_t j = 0; j < swapchainCount; ++j)
        {
            _imageAvailableSemaphores[j][i] = _device.device().createSemaphore(semaphoreCreateInfo);
            _renderFinishSemaphores[j][i] = _device.device().createSemaphore(semaphoreCreateInfo);
        }
        _inFlightFences[i] = _device.device().createFence(fenceCreateInfo);
    }
}

void RenderDelegator::draw()
{
    if (!_commandRecordFunc)
        return;
    (void)_device.device().waitForFences(_inFlightFences[_currentFrame], vk::True, UINT64_MAX);
    size_t swapchainCount = _pSwapchains.size();
    std::vector<uint32_t> imageIndices(swapchainCount);
    std::vector<vk::Semaphore> acquireSemaphores(swapchainCount);
    for (size_t i = 0; i < swapchainCount; ++i)
    {
        auto &pSwapchain = _pSwapchains[i];
        vk::Semaphore acquireSemaphore = _imageAvailableSemaphores[i][_currentFrame];
        vk::ResultValue<uint32_t> uintResult =
            _device.device().acquireNextImageKHR(pSwapchain->swapchain(), std::numeric_limits<uint64_t>::max(), acquireSemaphore, nullptr);
        imageIndices[i] = uintResult.value;
        acquireSemaphores[i] = acquireSemaphore;
        if (uintResult.result == vk::Result::eErrorOutOfDateKHR)
        {
            // 处理重建
            recreateSwapchain(i);
            return;
        }
        else if (uintResult.result != vk::Result::eSuccess && uintResult.result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }
    }
    _device.device().resetFences(_inFlightFences[_currentFrame]);
    vk::CommandBuffer &commandBuffer = _commandBuffers[_currentFrame];
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse).setPInheritanceInfo(nullptr);
    commandBuffer.reset();
    commandBuffer.begin(beginInfo);
    // 这里只传第一个 swapchain 的 imageIndex，实际可根据需求扩展
    _commandRecordFunc(commandBuffer, imageIndices[0], _currentFrame);
    commandBuffer.end();
    // 多 swapchain 时，waitSemaphores 取所有 acquireSemaphores
    std::vector<vk::Semaphore> waitSemaphores = acquireSemaphores;
    std::vector<vk::PipelineStageFlags> waitStages(waitSemaphores.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput);
    // signalSemaphores 取所有 renderFinishSemaphores
    std::vector<vk::Semaphore> signalSemaphores(swapchainCount);
    for (size_t i = 0; i < swapchainCount; ++i)
    {
        signalSemaphores[i] = _renderFinishSemaphores[i][_currentFrame];
    }
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer).setSignalSemaphores(signalSemaphores).setWaitDstStageMask(waitStages).setWaitSemaphores(waitSemaphores);
    try
    {
        _device.graphicsQueue().submit(submitInfo, _inFlightFences[_currentFrame]);
    }
    catch (const vk::SystemError &err)
    {
        std::cerr << "Vulkan error: " << err.what() << std::endl;
    }

    std::vector<vk::SwapchainKHR> vkSwapchains;
    for (size_t i = 0; i < swapchainCount; ++i)
        vkSwapchains.push_back(_pSwapchains[i]->swapchain());

    vk::PresentInfoKHR presentInfo;
    presentInfo.setImageIndices(imageIndices).setSwapchains(vkSwapchains).setWaitSemaphores(signalSemaphores);
    vk::Result presentResult = _device.presentQueue().presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || _isSizeChanged)
    {
        _isSizeChanged = false;
        // 处理重建
        recreateSwapchain(-1);
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