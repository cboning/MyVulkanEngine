#pragma once
#include "FrameGraphResource.h"
#include <string>
#include <vector>

namespace FrameGraph
{
class Pass
{
    friend class PassBuilder;
    friend class FrameGraph;

private:
    const std::string _name;
    std::vector<FrameGraphResource::Handle> _reads;
    std::vector<FrameGraphResource::Handle> _writes;

    Pass(const std::string &name);

public:
    ~Pass();
};
} // namespace FrameGraph