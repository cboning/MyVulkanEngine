#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "ResourceManager.h"
#define DEBUG

namespace Vkbase
{
    class ResourceBase
    {
    private:
        inline static ResourceManager _resourceManager{};
        bool _locked = false;

        void useSuperresource(ResourceBase *pResource);
        void disuseSuperresource(ResourceBase *pResource);
    protected:
        std::string _name;
        const ResourceType _resourceType;
        std::vector<ResourceBase *> _pSubresources;
        std::vector<ResourceBase *> _pSuperresources;
        inline static uint32_t _nameId = 0;
        void useSubresource(ResourceBase *pResource);
        void disusedSubresource(ResourceBase *pResource);
        template <typename T>
        T *connectTo(T *pResource)
        {
            if (pResource == this)
                return pResource;
            ResourceBase *pBase = (ResourceBase *)pResource;
            if (!pBase) {
                throw std::runtime_error("Invalid type: not the expected derived class");
            }
            pBase->useSuperresource(this);
            useSubresource(pBase);
            return pResource;
        }

    public:
        ResourceBase(ResourceType resourceType, const std::string &resourceName);
        virtual ~ResourceBase();
        static ResourceManager &resourceManager();
        const std::string &name() const;
        const ResourceType &type() const;
        void destroy() const;
        void preDestroy();
        void rename(const std::string &name);
        void setLock();
        void setUnlock();
    };
}