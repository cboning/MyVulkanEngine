#pragma once
#include "../../Vkbase/VkResourcesDelegator.h"
#include "../../Vkbase/VkResourceManagerHolder.h"
#include <string>
#include <unordered_map>

namespace FrameGraph
{
class Pool : public Vkbase::VkResourcesDelegator
{
private:
    struct Group{
        std::vector<Vkbase::VkResourceManagerHolder::WeakReference> resources;
        uint32_t unuseCount = 0;
    };
    const std::string _deviceName;
    std::vector<Group> _pool;

public:
    Pool(const std::string &deviceName);
    ~Pool();
};
} // namespace FrameGraph