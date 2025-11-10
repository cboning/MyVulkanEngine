#include "PassBuilder.h"
#include "Pass.h"

namespace FrameGraph
{
PassBuilder::PassBuilder(Pass &pass) : _pass(pass) {}
PassBuilder::~PassBuilder() {}

void PassBuilder::read(const FrameGraphResource::Handle &resourceName) { _pass._reads.push_back(resourceName); }

void PassBuilder::write(const FrameGraphResource::Handle &resourceName) { _pass._writes.push_back(resourceName); }

} // namespace FrameGraph