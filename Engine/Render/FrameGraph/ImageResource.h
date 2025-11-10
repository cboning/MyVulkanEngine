#pragma once
#include "FrameGraphResource.h"
#include <vulkan/vulkan.hpp>

namespace FrameGraph
{
class ImageResource : public FrameGraphResource
{
    friend class FrameGraph;
public:
    struct ImageDescription
    {
        uint32_t width, height;
        vk::Format format;
        uint32_t samples = 1;
    };
private:
    const ImageDescription _imageDescription;

    ImageResource(const ImageDescription &imageDescription);
    const ImageDescription &description() const;
};
} // namespace FrameGraph