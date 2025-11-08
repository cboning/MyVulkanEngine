#include "EmptyTextureResource.h"
#include "../Vkbase/Image.h"

namespace Resources
{
Vkbase::VkResourceManagerHolder::WeakReference EmptyTextureResource::texture() const
{
    return Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Image, _textureName);
}

EmptyTextureResource::EmptyTextureResource(const std::string &deviceName) : ResourceBase(ResourceType::EmptyTexture, getNameByArgument(deviceName))
{
    _textureName = createResource<Vkbase::Image>(_textureName, deviceName, 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D,
                                                 vk::ImageUsageFlagBits::eSampled, (uint32_t[]){0xFFFF00FF})
                       .lock()
                       ->name();
}

EmptyTextureResource::~EmptyTextureResource() {}

std::string EmptyTextureResource::getNameByArgument(const std::string &deviceName) { return deviceName + "_Empty"; }

} // namespace Resources