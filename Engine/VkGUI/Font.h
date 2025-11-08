#pragma once
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Image.h"
#include "../Vkbase/Sampler.h"
#include "../Vkbase/VkResourcesDelegator.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>
namespace Vkbase
{
} // namespace Vkbase

namespace VkGUI
{
class Font : public Vkbase::VkResourcesDelegator
{
private:
    struct Character : public Vkbase::VkResourcesDelegator
    {
        Character(const std::string &deviceName, const FT_Face &face, FT_ULong character)
            : imageName(createCharacterImage(deviceName, face, character)), size(glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows)),
              bearing(glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)), advance(face->glyph->advance.x)
        {
        }

        const std::string imageName;
        glm::ivec2 size;
        glm::ivec2 bearing;
        uint32_t advance;

    private:
        const std::string createCharacterImage(const std::string &deviceName, const FT_Face &face, FT_ULong character)
        {
            if (FT_Load_Char(face, character, FT_LOAD_RENDER) || face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0)
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                return "Empty";
            }

            return createResource<Vkbase::Image>(deviceName + std::string(face->style_name) + std::string((const char *)&character), deviceName,
                                                 face->glyph->bitmap.width, face->glyph->bitmap.rows, 1, vk::Format::eR8Unorm, vk::ImageType::e2D,
                                                 vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled, face->glyph->bitmap.buffer)
                .lock()
                ->name();
        }
    };
    const std::string _deviceName;
    std::unordered_map<FT_ULong, Character> _characters;

    FT_Library _ft;
    FT_Face _face;
    const uint32_t _fontSize;

    void loadCharacter(FT_ULong character);

public:
    Font(const std::string &deviceName, const std::string &filename, uint32_t fontSize);
    ~Font();
    const std::string &deviceName() const;
    const Font::Character &character(FT_ULong character);
    const std::string &characterTextureName(FT_ULong character);
};
} // namespace VkGUI