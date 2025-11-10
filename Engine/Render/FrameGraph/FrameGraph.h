#pragma once
#include "Pass.h"
#include "ImageResource.h"
#include "PassBuilder.h"
#include "Pool.h"
#include <functional>
#include <unordered_map>

namespace FrameGraph
{
class FrameGraph
{
private:
    std::unordered_map<std::string, Pass> _passes;
    std::vector<ImageResource> _imageResources;
    Pool _pool;

public:
    FrameGraph(const std::string &deviceName);;
    ~FrameGraph() = default;

    void addPass(const std::string &name, const std::function<void(PassBuilder &builder)> &setup, const std::function<void()> &execute);
    FrameGraphResource::Handle addResourceDescription(const ImageResource::ImageDescription &description);

    void compile();
};
} // namespace FrameGraph