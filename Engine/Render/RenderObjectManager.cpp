#include "RenderObjectManager.h"
#include "../Vkbase/RenderObjectDelegator.h"

void RenderObjectManager::addObject(const std::string &renderPassName, const std::string &pipelineName,
                                    const std::shared_ptr<Vkbase::RenderObjectDelegator> &object)
{
    _pRenderObjects[renderPassName][pipelineName].insert(std::weak_ptr<Vkbase::RenderObjectDelegator>(object));
}

void RenderObjectManager::addObject(const std::string &renderPassName, const std::string &pipelineName,
                                    const std::weak_ptr<Vkbase::RenderObjectDelegator> &object)
{
    _pRenderObjects[renderPassName][pipelineName].insert(object);
}

void RenderObjectManager::draw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                               uint32_t frameIndex)
{
    auto renderPassIter = _pRenderObjects.find(renderPassName);
    if (renderPassIter == _pRenderObjects.end())
        return;
    auto pipelineIter = renderPassIter->second.find(pipelineName);
    if (pipelineIter == renderPassIter->second.end())
        return;

    auto &renderObjects = pipelineIter->second;

    for (auto iter = renderObjects.begin(); iter != renderObjects.end();)
    {
        const auto &weakObject = *iter;
        if (weakObject.expired())
        {
            iter = renderObjects.erase(iter);
            continue;
        }

        auto sharedObject = weakObject.lock();
        if (auto sharedObject = weakObject.lock())
            sharedObject->draw(commandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);

        ++iter;
    }
}
