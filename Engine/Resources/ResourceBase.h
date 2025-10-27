#pragma once
#include "ResourceManager.h"

namespace Resources
{
class ResourceBase
{
    friend class ResourceManager;

public:
    ResourceType type() const;
    const std::string &name() const;

protected:
    ResourceBase(ResourceType type, const std::string &name);
    virtual ~ResourceBase();

private:
    ResourceType _type;
    const std::string _name;
    inline static ResourceManager &_reosurceManager = ResourceManager::instance();
};
} // namespace Resources
