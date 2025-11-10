#include "FrameGraphResource.h"
namespace FrameGraph
{

FrameGraphResource::FrameGraphResource(ResourceType type) : _type(type) {}

FrameGraphResource::~FrameGraphResource() {}

ResourceType FrameGraphResource::type() const { return _type; }

} // namespace FrameGraph
