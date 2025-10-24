#include "RenderPassManager.h"
#include "../Vkbase/CommandBuffer.h"
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

void RenderPassManager::draw(Vkbase::CommandBuffer *pCommandBuffer, RenderObjectManager *pObjects, uint32_t imageIndex, uint32_t frameIndex) const
{
    for (const RenderPassInfo &renderPass : _renderPasses)
    {
        Vkbase::RenderPass *pRenderPass =
            dynamic_cast<Vkbase::RenderPass *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::RenderPass, renderPass.name));
        pRenderPass->begin(pCommandBuffer, renderPass.name + "_" + std::to_string(imageIndex), renderPass.clearValues);
        bool firstPass = true;
        for (const std::vector<std::string> &pipelineNames : renderPass.pipelineNames)
        {
            if (firstPass)
                firstPass = false;
            else
                pCommandBuffer->commandBuffer().nextSubpass(vk::SubpassContents::eInline);

            for (const std::string &pipelineName : pipelineNames)
            {
                pCommandBuffer->bindPipeline(
                    dynamic_cast<Vkbase::Pipeline *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Pipeline, pipelineName)));

                pObjects->draw(pCommandBuffer, renderPass.name, pipelineName, imageIndex, frameIndex);
            }
        }
        pRenderPass->end(pCommandBuffer);
    }
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
