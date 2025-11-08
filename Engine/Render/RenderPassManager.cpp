#include "RenderPassManager.h"
#include "../Vkbase/CommandBuffer.h"
#include "../Vkbase/Framebuffer.h"
#include "../Vkbase/Pipeline.h"
#include "../Vkbase/RenderPass.h"
#include "RenderObjectManager.h"

std::vector<RenderPassManager::RenderPassInfo> RenderPassManager::processConfig(const json &config)
{
    std::vector<RenderPassInfo> result;
    result.reserve(config.size());

    for (const json &renderPass : config)
        result.push_back({renderPass["renderPassName"], renderPass["pipelineNames"], std::vector<vk::ClearValue>(renderPass["attachmentCount"])});
    return result;
}

RenderPassManager::RenderPassManager(const json &config) : _renderPasses(processConfig(config)) {}

RenderPassManager::~RenderPassManager()
{
    for (auto pipeline : _secondaryCommandBuffers)
        for (auto &[shouldRecord, secondaryCommandBuffer] : pipeline.second)
            if (auto p = secondaryCommandBuffer.lock<Vkbase::CommandBuffer>())
                p->destroy();
}

void RenderPassManager::draw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, RenderObjectManager *pObjects, uint32_t imageIndex,
                             uint32_t frameIndex)
{
    if (auto pCommandBuffer = commandBuffer.lock<Vkbase::CommandBuffer>())
    {
        const std::string &commandPoolName = pCommandBuffer->commandPoolName();

        for (const RenderPassInfo &renderPass : _renderPasses)
        {
            const std::string framebufferName = renderPass.name + "_" + std::to_string(imageIndex);

            // Get render pass once before loop
            if (auto pRenderPass =
                    Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::RenderPass, renderPass.name).lock<Vkbase::RenderPass>())
            {
                // Pre-collect command buffers for execution
                std::vector<Vkbase::VkResourceManagerHolder::WeakReference> secondaryBuffers;
                secondaryBuffers.reserve(renderPass.pipelineNames.size() * 2); // Approximate size

                pRenderPass->begin(commandBuffer, framebufferName, renderPass.clearValues, vk::SubpassContents::eSecondaryCommandBuffers);

                for (size_t i = 0; i < renderPass.pipelineNames.size(); ++i)
                {
                    if (i > 0)
                        pCommandBuffer->commandBuffer().nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);

                    secondaryBuffers.clear();
                    for (const std::string &pipelineName : renderPass.pipelineNames[i])
                    {
                        auto buffer = recordSecondaryBuffer(commandPoolName, pObjects, framebufferName, renderPass.name, pipelineName, imageIndex, frameIndex);
                        secondaryBuffers.push_back(buffer);
                    }

                    if (!secondaryBuffers.empty())
                        pCommandBuffer->executeCommands(secondaryBuffers);
                }

                pRenderPass->end(commandBuffer);
            }
        }
    }
}

void RenderPassManager::registSecondaryBuffer(const std::string &commandPoolName, const std::string &pipelineName, uint32_t frameIndex)
{
    if (!_secondaryCommandBuffers.count(pipelineName))
        _secondaryCommandBuffers.insert({pipelineName, {}});

    std::vector<std::pair<bool, Vkbase::VkResourceManagerHolder::WeakReference>> &secondaryCommandBuffers = _secondaryCommandBuffers[pipelineName];

    if (secondaryCommandBuffers.size() <= frameIndex)
        secondaryCommandBuffers.resize(frameIndex + 1);

    if (secondaryCommandBuffers[frameIndex].second.lock())
        return;

    if (auto pCommandPool = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::CommandPool, commandPoolName).lock<Vkbase::CommandPool>())
        secondaryCommandBuffers[frameIndex].second = pCommandPool->allocateSecondaryCommandBuffers(1)[0];
}

Vkbase::VkResourceManagerHolder::WeakReference RenderPassManager::recordSecondaryBuffer(const std::string &commandPoolName, RenderObjectManager *pObjects,
                                                                                        const std::string &framebufferName, const std::string &renderPassName,
                                                                                        const std::string &pipelineName, uint32_t imageIndex,
                                                                                        uint32_t frameIndex)
{
    if (!pObjects)
        throw std::invalid_argument("pObjects is null");

    registSecondaryBuffer(commandPoolName, pipelineName, frameIndex);
    auto &entry = _secondaryCommandBuffers[pipelineName][frameIndex];
    auto &state = entry.first;

    if (state && entry.second.lock<Vkbase::CommandBuffer>())
        return entry.second;

    if (!entry.second.lock<Vkbase::CommandBuffer>())
        throw std::runtime_error("Failed to allocate secondary command buffer for pipeline: " + pipelineName);

    state = true;

    auto &rm = Vkbase::VkResourceManager::instance();
    auto pipeline = rm.resource(Vkbase::VkResourceType::Pipeline, pipelineName);
    if (auto pPipeline = pipeline.lock<Vkbase::Pipeline>())
    {
        if (auto pFramebuffer = rm.resource(Vkbase::VkResourceType::Framebuffer, framebufferName).lock<Vkbase::Framebuffer>())
        {
            if (auto pCommandBuffer = entry.second.lock<Vkbase::CommandBuffer>())
            {
                pCommandBuffer->reset();
                // Begin recording for the correct subpass and set viewport/scissor once
                pCommandBuffer->begin(renderPassName, pPipeline->subpass());
                Vkbase::RenderPass::setViewportScissor(entry.second, pFramebuffer->extent());

                pCommandBuffer->bindPipeline(pipeline);
                pObjects->draw(entry.second, renderPassName, pipelineName, imageIndex, frameIndex);

                pCommandBuffer->end();

                return entry.second;
            }
            else
                throw std::runtime_error("CommandBuffer not found.");
        }
        else
            throw std::runtime_error("Framebuffer not found: " + framebufferName);
    }
    else
        throw std::runtime_error("Pipeline not found: " + pipelineName);
}

void RenderPassManager::shouldRecordFor(const std::string &pipelineName)
{
    if (!_secondaryCommandBuffers.count(pipelineName))
        return;

    for (auto &[state, commandBuffer] : _secondaryCommandBuffers[pipelineName])
    {
        state = false;
    }
}

void RenderPassManager::shouldRecordFor()
{
    for (auto &[name, t] : _secondaryCommandBuffers)
        shouldRecordFor(name);
}

std::vector<vk::ClearValue> &RenderPassManager::clearValue(const std::string &renderPassName)
{
    for (RenderPassInfo &renderPass : _renderPasses)
    {
        if (renderPass.name == renderPassName)
            return renderPass.clearValues;
    }
    throw std::runtime_error("Index out of range.");
}
