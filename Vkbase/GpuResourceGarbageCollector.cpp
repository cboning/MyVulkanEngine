#include "GpuResourceGarbageCollector.h"

namespace Vkbase
{

GpuResourceGarbageCollector::~GpuResourceGarbageCollector() { forceCollect(); }

void GpuResourceGarbageCollector::defer(std::unique_ptr<uint32_t> &counter, std::function<void()> deleter) { _pending.emplace_back(std::move(counter), std::move(deleter)); }

void GpuResourceGarbageCollector::collect()
{
    auto it = _pending.begin();
    while (it != _pending.end())
    {
        if (!it->first || !*it->first)
        {
            if (it->second)
                it->second();
            it = _pending.erase(it);
        }
        else
            return;
    }
}

void GpuResourceGarbageCollector::forceCollect()
{
    for (auto &[fence, deleter] : _pending)
        if (deleter)
            deleter();
    _pending.clear();
}

bool GpuResourceGarbageCollector::hasPending() const { return !_pending.empty(); }

} // namespace Vkbase
