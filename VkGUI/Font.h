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
class Buffer;
class CommandBuffer;
} // namespace Vkbase
class Text;
class Font : public Vkbase::VkResourcesDelegator
{
private:
    class projectiveUniformBuffer : public Vkbase::VkResourcesDelegator
    {
    private:
        const std::string deviceName;
        Vkbase::Buffer *_pUBO = nullptr;

    public:
        projectiveUniformBuffer(const std::string &deviceName);
        Vkbase::Buffer *getUBO();
    };
    struct Character : public Vkbase::VkResourcesDelegator
    {
        Character(const std::string &deviceName, const FT_Face &face, FT_ULong character)
            : imageName(createCharacterImage(deviceName, face, character)), size(glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows)),
              bearing(glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)), advance(face->glyph->advance.x)
        {
        }

        Vkbase::Image &image() { return *dynamic_cast<Vkbase::Image *>(Vkbase::Image::resourceManager().resource(Vkbase::VkResourceType::Image, imageName)); }

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

            return (createResource<Vkbase::Image>(deviceName + std::string(face->style_name) + std::string((const char *)&character), deviceName,
                                                  face->glyph->bitmap.width, face->glyph->bitmap.rows, 1, vk::Format::eR8Unorm, vk::ImageType::e2D,
                                                  vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled, face->glyph->bitmap.buffer))
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
    std::unordered_set<std::shared_ptr<Text>> _pTexts;
    inline static std::unique_ptr<projectiveUniformBuffer> _pProjectiveUniformBuffer = nullptr;

public:
    Font(const std::string &deviceName, const std::string &filename);
    ~Font() = default;
    const std::string &deviceName() const;
    const std::unordered_map<FT_ULong, Character> &characters() const;
    std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>> set(FT_ULong character) const;

    std::weak_ptr<Text> createText();
    std::weak_ptr<Text> createText(const std::string &text, const glm::vec3 &color, const glm::vec2 &pos, float scale);
    void removeText(std::weak_ptr<Text> &pText);
    const vk::DescriptorSetLayout &layout() const;
    static void addProjectiveDescriptorSet(const std::string &descriptorSetsName);
    static void writeProjectiveDescriptorSet(const std::string &descriptorSetsName, const std::string &deviceName);
    static void setScreenSize(const glm::ivec2 &screenSize);
    static const vk::DescriptorSetLayout &projectiveLayout(const std::string &descriptorSetsName);
    static std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>> projectiveSet(const std::string &descriptorSetsName);
};