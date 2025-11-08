#pragma once

#include "ResourceBase.h"
namespace VkGUI
{
class Font;
}

namespace Resources
{
class FontResource : public ResourceBase
{
    friend class ResourceManager;

public:
    VkGUI::Font *font() const;

private:
    std::shared_ptr<VkGUI::Font> _pFont;

    FontResource(const std::string &deviceName, const std::string &filename, uint32_t fontSize);
    ~FontResource() override;

    static std::string getNameByArgument(const std::string &deviceName, const std::string &filename, uint32_t fontSize);
};
} // namespace Resources
