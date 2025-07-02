#include "RenderDelegator.h"
#include "Device.h"
#include "Swapchain.h"
#include "CommandPool.h"
#define MAX_FLIGHT_COUNT 3

namespace Vkbase
{
    RenderDelegator::RenderDelegator(const std::string &resourceName, const std::string &deviceName, const std::string &renderPassName, const std::string &swapchainName, const std::string &commandPoolName)
        : ResourceBase(Vkbase::ResourceType::RenderDelegator, resourceName), _device(*dynamic_cast<const Device *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Device, deviceName)))), _swapchain(*dynamic_cast<const Swapchain *>(connectTo(resourceManager().resource(Vkbase::ResourceType::Swapchain, swapchainName)))), _commandPool(*dynamic_cast<const CommandPool *>(connectTo(resourceManager().resource(Vkbase::ResourceType::CommandPool, commandPoolName))))
    {
        init();
    }

    RenderDelegator::~RenderDelegator()
    {
        _commandPool.freeCommandBuffers(_commandBuffers);

        for (const vk::Semaphore &semaphore : _renderFinishSemaphores)
            _device.device().destroySemaphore(semaphore);
        _renderFinishSemaphores.clear();

        for (const vk::Semaphore &semaphore : _imageAvailableSemaphores)
            _device.device().destroySemaphore(semaphore);
        _imageAvailableSemaphores.clear();

        for (const vk::Fence &fence : _inFlightFences)
            _device.device().destroyFence(fence);
        _inFlightFences.clear();
    }

    void RenderDelegator::init()
    {
        _commandBuffers = _commandPool.allocateFlightCommandBuffers(MAX_FLIGHT_COUNT);
        createSyncObjects();
    }

    void RenderDelegator::createSyncObjects()
    {
        _imageAvailableSemaphores.resize(MAX_FLIGHT_COUNT);
        _renderFinishSemaphores.resize(MAX_FLIGHT_COUNT);
        _inFlightFences.resize(MAX_FLIGHT_COUNT);

        vk::SemaphoreCreateInfo semaphoreCreateInfo;
        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        for (int i = 0; i < MAX_FLIGHT_COUNT; i++)
        {
            _imageAvailableSemaphores[i] = _device.device().createSemaphore(semaphoreCreateInfo);
            _renderFinishSemaphores[i] = _device.device().createSemaphore(semaphoreCreateInfo);
            _inFlightFences[i] = _device.device().createFence(fenceCreateInfo);
        }
    }

    void RenderDelegator::draw()
    {
        if (!_commandRecordFunc)
            return ;
        
        _device.device().waitForFences(_inFlightFences[_currentFrame], vk::True, UINT64_MAX);
        vk::ResultValue<uint32_t> uintResult = _device.device().acquireNextImageKHR(_swapchain.swapchain(), std::numeric_limits<uint64_t>::max(), _imageAvailableSemaphores[_currentFrame]);
        uint32_t imageIndex = uintResult.value;
        if (uintResult.result == vk::Result::eErrorOutOfDateKHR)
        {
            
        }
        else if (uintResult.result != vk::Result::eSuccess && uintResult.result != vk::Result::eSuboptimalKHR)
            throw std::runtime_error("Failed to acquire swap chain image!");

        _device.device().resetFences(_inFlightFences[_currentFrame]);
        vk::CommandBuffer &commandBuffer = _commandBuffers[_currentFrame];
        commandBuffer.reset();

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
            .setPInheritanceInfo(nullptr);

        commandBuffer.begin(beginInfo);

        _commandRecordFunc(resourceManager(), commandBuffer, imageIndex);

        commandBuffer.end(); // 结束命令记录

        std::vector<vk::Semaphore> waitSemaphores = {_imageAvailableSemaphores[_currentFrame]};
        std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        std::vector<vk::Semaphore> signalSemaphores = {_renderFinishSemaphores[_currentFrame]};

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer)
            .setSignalSemaphores(signalSemaphores)
            .setWaitDstStageMask(waitStages)
            .setWaitSemaphores(waitSemaphores);

        _device.graphicsQueue().submit(submitInfo, _inFlightFences[_currentFrame]);

        vk::SwapchainKHR swapChains[] = {_swapchain.swapchain()};

        vk::PresentInfoKHR presentInfo;
        presentInfo.setImageIndices(imageIndex)
            .setSwapchains(swapChains)
            .setWaitSemaphores(signalSemaphores);

        vk::Result presentResult = _device.presentQueue().presentKHR(presentInfo);

        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || _isSizeChanged)
        {
            _isSizeChanged = false;
            // _pSurface->swapChain()->recreateSwapChain();
        }
        else if (presentResult != vk::Result::eSuccess)
            throw std::runtime_error("Failed to present swap chain image!");
        _currentFrame = (_currentFrame + 1) % MAX_FLIGHT_COUNT;
    }

    void RenderDelegator::sizeChanged()
    {
        _isSizeChanged = true;
    }

    void RenderDelegator::setCommandRecordFunc(void (* recordFunc)(ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex))
    {
        _commandRecordFunc = recordFunc;
    }
}