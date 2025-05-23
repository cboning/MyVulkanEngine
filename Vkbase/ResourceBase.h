#pragma once
#include <string>
#include "ResourceManager.h"

namespace Vkbase
{
    enum class ResourceType
    {
        Window,
        Swapchain,
        Device,
        Image,
        Buffer,
        Pipeline,
        Framebuffer,
        CommandPool,

    };

    class ResourceBase
    {
    private:
        inline static ResourceManager _resourceManager{};

    protected:
        ResourceType _resourceType;
        std::string _name;

    public:
        ResourceBase(const std::string &resourceName, ResourceType resourceType);
        virtual ~ResourceBase() = default;
        static ResourceManager &resourceManager();
    };
}