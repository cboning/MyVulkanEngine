#pragma once
#include "../../Vkbase/VkResourceManagerHolder.h"
#include <cstdint>

namespace FrameGraph
{

enum class ResourceType
{
    Image,
    FrameBuffer,
    RenderPass,
    Pipeline,
    Buffer
};

class FrameGraphResource
{
private:
    ResourceType _type;

public:
    class Handle
    {
    private:
        friend class FrameGraph;
        uint32_t _id;
        Handle(uint32_t id) : _id(id) {}
    };
    FrameGraphResource(ResourceType type);
    ~FrameGraphResource();

    ResourceType type() const;
};

} // namespace FrameGraph