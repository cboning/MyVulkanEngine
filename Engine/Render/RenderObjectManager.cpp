#include "RenderObjectManager.h"
#include "../Vkbase/RenderObjectDelegator.h"

void RenderObjectManager::addObject(const std::string &renderPassName, const std::string &pipelineName, const std::shared_ptr<Vkbase::RenderObjectDelegator> &object)
{
    _pRenderObjects[renderPassName][pipelineName].insert(std::weak_ptr<Vkbase::RenderObjectDelegator>(object));
}

void RenderObjectManager::addObject(const std::string &renderPassName, const std::string &pipelineName, const std::weak_ptr<Vkbase::RenderObjectDelegator> &object)
{
    _pRenderObjects[renderPassName][pipelineName].insert(object);
}

void RenderObjectManager::draw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                               uint32_t frameIndex)
{
    std::unordered_set<std::weak_ptr<Vkbase::RenderObjectDelegator>, WeakPtrHash, WeakPtrEqual> &objects = _pRenderObjects[renderPassName][pipelineName];
    std::unordered_set<std::weak_ptr<Vkbase::RenderObjectDelegator>, WeakPtrHash, WeakPtrEqual> eraseObjects;
    for (auto &object : objects)
    {
        if (object.expired())
        {
            eraseObjects.insert(object);
            continue;
        }

        if (auto pObject = object.lock())
            pObject->draw(pCommandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);
    }

    for (auto &eraseObject : eraseObjects)
        objects.erase(eraseObject);
}


