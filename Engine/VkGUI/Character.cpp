#include "Character.h"
#include "Font.h"

namespace VkGUI
{
Character::Character(const std::string &deviceName, Font *pFont, char32_t character)
    : Widget(WidgetType::Character, deviceName, pFont->characterTextureName(character))
{
}
} // namespace VkGUI