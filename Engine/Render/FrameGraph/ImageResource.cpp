#include "ImageResource.h"

namespace FrameGraph
{
ImageResource::ImageResource(const ImageDescription &imageDescription)
    : FrameGraphResource(ResourceType::Image), _imageDescription(imageDescription)
{
}
const ImageResource::ImageDescription &ImageResource::description() const { return _imageDescription; }
} // namespace FrameGraph