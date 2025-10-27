#include "ResourceBase.h"
namespace Resources
{
ResourceType Resources::ResourceBase::type() const { return _type; }

const std::string &Resources::ResourceBase::name() const { return _name; }

ResourceBase::ResourceBase(ResourceType type, const std::string &name) : _type(type), _name(name) { _reosurceManager.addResource(this); }

ResourceBase::~ResourceBase() {}
} // namespace Resources