#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "ResourceManager.h"

namespace Vkbase
{
    class ResourceBase
    {
        friend class ResourceManager;
    private:
        inline static ResourceManager _resourceManager{};
        bool _locked = false;

        void useSuperresource(ResourceBase *pResource);
        void disuseSuperresource(ResourceBase *pResource);
    protected:
        ResourceBase(ResourceType resourceType, const std::string &resourceName);
        std::string _name;
        const ResourceType _resourceType;
        std::vector<ResourceBase *> _pSubresources;
        std::vector<ResourceBase *> _pSuperresources;
        void useSubresource(ResourceBase *pResource);
        void disusedSubresource(ResourceBase *pResource);
        static std::string getSuitableName(const ResourceType &type, std::string name);
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