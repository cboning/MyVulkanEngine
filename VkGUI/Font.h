#pragma once
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Image.h"
#include "../Vkbase/Sampler.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>
class Font
{

  private:
    struct Character
    {
        Character(const std::string &deviceName, const FT_Face &face, FT_ULong character)
            : imageName(createCharacterImage(deviceName, face, character)), size(glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows)),
              bearing(glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)), advance(face->glyph->advance.x)
        {
        }

        const Vkbase::Image &image()
        {
            return *dynamic_cast<const Vkbase::Image *>(Vkbase::Image::resourceManager().resource(Vkbase::ResourceType::Image, imageName));
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

            return (Vkbase::ResourceBase::resourceManager().create<Vkbase::Image>(std::string(face->style_name) + std::string((const char *)&character), deviceName, face->glyph->bitmap.width,
                                      face->glyph->bitmap.rows, 1, vk::Format::eR8Unorm, vk::ImageType::e2D, vk::ImageViewType::e2D,
                                      vk::ImageUsageFlagBits::eSampled, face->glyph->bitmap.buffer))
                ->name();
        }
    };

    struct UniformBufferData
    {
        glm::mat4 projection;
    };

    const std::string _deviceName;
    Vkbase::Sampler &_sampler;
    Vkbase::DescriptorSets &_descriptorSets;
    std::unordered_map<FT_ULong, Character> _characters;

  public:
    Font(const std::string &deviceName, const std::string &filename);
    ~Font();
    const std::string &deviceName() const;
    const std::unordered_map<FT_ULong, Character> &characters() const;
    const vk::DescriptorSet &set(FT_ULong character) const;
    const vk::DescriptorSetLayout &layout() const;
    static void addProjectiveDescriptorSet(const std::string &descriptorSetsName);
    static void writeProjectiveDescriptorSet(const std::string &descriptorSetsName, const std::string &deviceName);
    static void setScreenSize(const glm::ivec2 &screenSize);
    static const vk::DescriptorSetLayout &projectiveLayout(const std::string &descriptorSetsName);
    static const vk::DescriptorSet &projectiveSet(const std::string &descriptorSetsName);
};