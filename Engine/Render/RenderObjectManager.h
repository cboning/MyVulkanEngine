#pragma once

#include "../Vkbase/VkResourceManagerHolder.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Vkbase
{
class RenderObjectDelegator;
} // namespace Vkbase

class RenderObjectManager
{
private:
    struct WeakPtrHash
    {
        template <typename T> size_t operator()(const std::weak_ptr<T> &wp) const noexcept
        {
            if (auto sp = wp.lock())
            {
                // 使用 shared_ptr 的地址哈希
                return std::hash<T *>()(sp.get());
            }
            // expired 时返回固定哈希（避免未定义行为）
            return 0;
        }
    };

    struct WeakPtrEqual
    {
        template <typename T> bool operator()(const std::weak_ptr<T> &a, const std::weak_ptr<T> &b) const noexcept
        {
            auto sp1 = a.lock();
            auto sp2 = b.lock();
            return sp1.get() == sp2.get();
        }
    };
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_set<std::weak_ptr<Vkbase::RenderObjectDelegator>, WeakPtrHash,
                                                                                       WeakPtrEqual>>>
        _pRenderObjects; // renderPassName, {pipelineName, objects}

public:
    void addObject(const std::string &renderPassName, const std::string &pipelineName, const std::shared_ptr<Vkbase::RenderObjectDelegator> &object);
    void addObject(const std::string &renderPassName, const std::string &pipelineName, const std::weak_ptr<Vkbase::RenderObjectDelegator> &object);

    void draw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName,
              uint32_t imageIndex, uint32_t frameIndex);
};