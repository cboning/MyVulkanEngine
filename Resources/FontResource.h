#pragma once

#include "ResourceBase.h"

class Font;

namespace Resources
{
class FontResource : public ResourceBase
{
    friend class ResourceManager;
public:
    Font *font() const;

private:
    std::shared_ptr<Font> _pFont;

    FontResource(const std::string &deviceName, const std::string &filename);
    ~FontResource() override;
    
    static std::string getNameByArgument(const std::string &deviceName, const std::string &filename);
};
} // namespace Resources
