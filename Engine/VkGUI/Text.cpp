#include "Text.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/CommandBuffer.h"
namespace VkGUI
{
Text::Text(const std::string &deviceName, Font *pFont) : Frame(deviceName), _pFont(pFont) {}

std::u32string Text::utf8_to_utf32(const std::string &str)
{
    std::u32string result;
    size_t i = 0;
    while (i < str.size())
    {
        uint32_t ch = 0;
        unsigned char c = str[i];
        if (c < 0x80)
        {
            ch = c;
            i += 1;
        }
        else if ((c >> 5) == 0x6)
        {
            ch = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F);
            i += 2;
        }
        else if ((c >> 4) == 0xE)
        {
            ch = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
            i += 3;
        }
        else if ((c >> 3) == 0x1E)
        {
            ch = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
            i += 4;
        }
        result.push_back(ch);
    }
    return result;
}

void Text::setText(const std::string &text)
{
    if (_text == text)
        return;
    _text = text;
    std::u32string text32 = utf8_to_utf32(_text);

    for (auto character : _characters)
        if (auto pCharacter = character.lock())
            pCharacter->destroy();

    _characters.clear();
    _characters.reserve(text32.size());

    float maxAscent = 0.0f;
    float maxDescent = 0.0f;
    float totalWidth = 0.0f;

    for (char32_t c : text32)
    {
        const auto &ch = _pFont->character(c);
        float ascent = ch.bearing.y;
        float descent = ch.size.y - ch.bearing.y;

        maxAscent = std::max(maxAscent, ascent);
        maxDescent = std::max(maxDescent, descent);
        totalWidth += (ch.advance >> 6);
    }

    float textHeight = maxAscent + maxDescent;

    setRect(glm::u32vec4(rect().x, rect().y, static_cast<uint32_t>(totalWidth), static_cast<uint32_t>(textHeight)));

    glm::vec2 currentPos = glm::vec2(0, maxAscent);

    for (char32_t c : text32)
    {
        auto &charData = _pFont->character(c);
        auto characterWidget = create<Character>(_pFont, c);

        glm::vec2 cpos = currentPos + glm::vec2(charData.bearing.x, -charData.bearing.y);

        if (auto pCharacterWidget = characterWidget.lock())
        {
            pCharacterWidget->setRect(glm::u32vec4(cpos, charData.size));
            pCharacterWidget->setColor(_textColor);
        }

        _characters.push_back(characterWidget);
        currentPos.x += (charData.advance >> 6);
    }
}
void Text::setTextColor(const glm::vec4 &color)
{
    _textColor = color;
    for (auto character : _characters)
        if (auto pCharacter = character.lock())
            pCharacter->setColor(_textColor);
}
} // namespace VkGUI