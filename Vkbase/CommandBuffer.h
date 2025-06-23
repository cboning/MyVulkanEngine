#pragma once
#include <vulkan/vulkan.hpp>
#include "ResourceBase.h"

namespace Vkbase
{
    class CommandBuffer : public ResourceBase
    {
    public:

    private:
        vk::CommandBuffer &_commandBuffer;

    };
}