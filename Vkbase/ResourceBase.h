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
        const ResourceType _resourceType;
        const std::string _name;
        std::unordered_set<ResourceBase *> _pSubresources;
        std::unordered_set<ResourceBase *> _pSuperresources;
        inline static uint32_t _nameId = 0;
        void useSubresource(ResourceBase *pResource);
        void disusedSubresource(ResourceBase *pResource);
        template <typename T>
        T *connectTo(T *pResource)
        {
            T *derived = dynamic_cast<T *>(pResource);
            if (!derived) {
                throw std::runtime_error("Invalid type: not the expected derived class");
            }
            pResource->useSuperresource(this);
            useSubresource(pResource);
            return derived;
        }

    public:
        ResourceBase(ResourceType resourceType, const std::string &resourceName);
        virtual ~ResourceBase();
        static ResourceManager &resourceManager();
        void useSuperresource(ResourceBase *pResource);
        void disuseSuperresource(ResourceBase *pResource);
        const std::string &name() const;
        const ResourceType &type() const;
        void disconnect();
    };
}