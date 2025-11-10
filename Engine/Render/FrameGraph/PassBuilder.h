#pragma once
#include "FrameGraphResource.h"
#include <string>

namespace FrameGraph
{
class Pass;
class PassBuilder
{
private:
    Pass &_pass;

public:
    PassBuilder(Pass &pass);
    ~PassBuilder();
    void read(const FrameGraphResource::Handle &resource);
    void write(const FrameGraphResource::Handle &resource);
};

} // namespace FrameGraph