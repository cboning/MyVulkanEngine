#include "FontResource.h"
#include "../VkGUI/Font.h"
namespace Resources
{
Font *FontResource::font() const { return _pFont.get(); }

FontResource::FontResource(const std::string &deviceName, const std::string &filename)
    : ResourceBase(ResourceType::Font, getNameByArgument(deviceName, filename))
{
    _pFont = std::make_shared<Font>(deviceName, filename);
    _pFont->setDestroyCallback([&]() { ResourceManager::instance().removeResource(this); });
}

FontResource::~FontResource() {}

std::string FontResource::getNameByArgument(const std::string &deviceName, const std::string &filename) { return deviceName + "_" + filename; }

} // namespace Resources