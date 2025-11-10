#include "FrameGraph.h"

namespace FrameGraph
{
FrameGraph::FrameGraph(const std::string &deviceName) : _pool(deviceName) {}

void FrameGraph::addPass(const std::string &name, const std::function<void(PassBuilder &builder)> &setup, const std::function<void()> &execute)
{
    if (_passes.count(name))
        return;

    PassBuilder builder(_passes.emplace(name, name).first->second);
    setup(builder);
}

FrameGraphResource::Handle FrameGraph::addResourceDescription(const ImageResource::ImageDescription &description)
{
    _imageResources.emplace_back(ImageResource(description));
    return FrameGraphResource::Handle(_imageResources.size());
}

void FrameGraph::compile()
{
    
}
}  // namespace FrameGraph