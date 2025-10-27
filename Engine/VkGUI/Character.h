#pragma once
#include "Widget.h"
namespace VkGUI
{
class Font;

class Character : public Widget
{
private:

public:
    Character(const std::string &deviceName, Font *pFont, char32_t character);
    ~Character() = default;
};

} // namespace VkGUI