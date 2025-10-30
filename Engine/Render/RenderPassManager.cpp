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
            secondaryCommandBuffer->destroy();
}

void RenderPassManager::draw(Vkbase::CommandBuffer *pCommandBuffer, RenderObjectManager *pObjects, uint32_t imageIndex, uint32_t frameIndex)
{
    for (const RenderPassInfo &renderPass : _renderPasses)
    {
        const std::string framebufferName = renderPass.name + "_" + std::to_string(imageIndex);
        Vkbase::RenderPass *pRenderPass =
            dynamic_cast<Vkbase::RenderPass *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::RenderPass, renderPass.name));
        pRenderPass->begin(pCommandBuffer, framebufferName, renderPass.clearValues, vk::SubpassContents::eSecondaryCommandBuffers);
        bool firstPass = true;
        for (const std::vector<std::string> &pipelineNames : renderPass.pipelineNames)
        {
            if (firstPass)
                firstPass = false;
            else
                pCommandBuffer->commandBuffer().nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);

            for (const std::string &pipelineName : pipelineNames)
                pCommandBuffer->executeCommands({recordSecondaryBuffer(pCommandBuffer->commandPoolName(), pObjects, framebufferName, renderPass.name,
                                                                       pipelineName, imageIndex, frameIndex)});
        }
        pRenderPass->end(pCommandBuffer);
    }
}

void RenderPassManager::registSecondaryBuffer(const std::string &commandPoolName, const std::string &pipelineName, uint32_t frameIndex)
{
    if (!_secondaryCommandBuffers.count(pipelineName))
        _secondaryCommandBuffers.insert({pipelineName, {}});

    std::vector<std::pair<bool, Vkbase::CommandBuffer *>> &secondaryCommandBuffers = _secondaryCommandBuffers[pipelineName];

    if (secondaryCommandBuffers.size() <= frameIndex)
        secondaryCommandBuffers.resize(frameIndex + 1);

    if (secondaryCommandBuffers[frameIndex].second)
        return;

    secondaryCommandBuffers[frameIndex].second =
        dynamic_cast<Vkbase::CommandPool *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::CommandPool, commandPoolName))
            ->allocateSecondaryCommandBuffers(1)[0];
}

Vkbase::CommandBuffer *RenderPassManager::recordSecondaryBuffer(const std::string &commandPoolName, RenderObjectManager *pObjects,
                                                                const std::string &framebufferName, const std::string &renderPassName,
                                                                const std::string &pipelineName, uint32_t imageIndex, uint32_t frameIndex)
{
    registSecondaryBuffer(commandPoolName, pipelineName, frameIndex);

    auto &[state, pCommandBuffer] = _secondaryCommandBuffers[pipelineName][frameIndex];

    if (state)
        return pCommandBuffer;

    state = true;
    Vkbase::Pipeline *pPipeline =
        dynamic_cast<Vkbase::Pipeline *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Pipeline, pipelineName));
    pCommandBuffer->begin(renderPassName, pPipeline->subpass());

    Vkbase::RenderPass::setViewportScissor(
        pCommandBuffer,
        dynamic_cast<Vkbase::Framebuffer *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Framebuffer, framebufferName))->extent());

    pCommandBuffer->bindPipeline(pPipeline);
    pObjects->draw(pCommandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);

    pCommandBuffer->end();

    return pCommandBuffer;
}

void RenderPassManager::shouldRecordFor(const std::string &pipelineName)
{
    if (!_secondaryCommandBuffers.count(pipelineName))
        return;

    for (auto &[state, pCommandBuffer] : _secondaryCommandBuffers[pipelineName])
    {
        if (state)
            pCommandBuffer->reset();
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
