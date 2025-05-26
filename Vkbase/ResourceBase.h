#pragma once
#include <string>
#include <unordered_set>
#include "ResourceManager.h"

namespace Vkbase
{
    class ResourceBase
    {
    private:
        inline static ResourceManager _resourceManager{};


    protected:
        ResourceType _resourceType;
        std::string _name;
        std::unordered_set<ResourceBase *> _pSubresources;
        std::unordered_set<ResourceBase *> _pSuperresources;
        std::unordered_set<std::string, ResourceBase*> _pResources;
        void useSubresource(ResourceBase *pResource);
        void useSuperresource(ResourceBase *pResource);
        
        void disusedSubresource(ResourceBase *pResource);
        void disuseSuperresource(ResourceBase *pResource);

    

    public:
        ResourceBase(ResourceType resourceType, const std::string &resourceName);
        virtual ~ResourceBase();
        static ResourceManager &resourceManager();
    };
}