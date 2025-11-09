#include "Font.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/DescriptorSets.h"
#include "Text.h"
#include <glm/gtc/matrix_transform.hpp>

namespace VkGUI
{
void Font::loadCharacter(FT_ULong character) { _characters.try_emplace(character, _deviceName, _face, character); }

Font::Font(const std::string &deviceName, const std::string &filename, uint32_t fontSize) : _deviceName(deviceName), _fontSize(fontSize)
{
    if (FT_Init_FreeType(&_ft))
    {
#ifdef DEBUG
        std::cout << "[Error] Could not init FreeType Library" << std::endl;
#endif
    }

    if (FT_New_Face(_ft, filename.c_str(), 0, &_face))
    {
#ifdef DEBUG
        std::cout << "[Error] Failed to load font" << std::endl;
#endif
    }
    FT_Set_Pixel_Sizes(_face, 0, _fontSize);
}

Font::~Font()
{
    FT_Done_Face(_face);
    FT_Done_FreeType(_ft);
}

const std::string &Font::deviceName() const { return _deviceName; }

const Font::Character &Font::character(FT_ULong character)
{
    loadCharacter(character);
    return _characters.at(character);
}

const std::string &Font::characterTextureName(FT_ULong character)
{
    loadCharacter(character);
    return _characters.at(character).imageName;
}
} // namespace VkGUI