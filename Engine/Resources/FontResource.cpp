#include "FontResource.h"
#include "../VkGUI/Font.h"

namespace Resources
{
VkGUI::Font *FontResource::font() const { return _pFont.get(); }

FontResource::FontResource(const std::string &deviceName, const std::string &filename, uint32_t fontSize)
    : ResourceBase(ResourceType::Font, getNameByArgument(deviceName, filename, fontSize))
{
    _pFont = std::make_shared<VkGUI::Font>(deviceName, filename, fontSize);
}

FontResource::~FontResource() {}

std::string FontResource::getNameByArgument(const std::string &deviceName, const std::string &filename, uint32_t fontSize)
{
    return deviceName + "_" + filename + "_" + std::to_string(fontSize);
}

} // namespace Resources