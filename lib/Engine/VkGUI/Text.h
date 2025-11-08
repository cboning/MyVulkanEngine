#pragma once
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Pipeline.h"
#include "Frame.h"
#include "Font.h"
#include "Character.h"
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class DescriptorSets;
}

namespace VkGUI
{
class Text : public Frame
{
    friend class Widget;
public:
    ~Text() = default;
    void setText(const std::string &text);
    void setTextColor(const glm::vec4 &color);

private:
    Text(const std::string &deviceName, Font *pFont);
    Font *_pFont = nullptr;
    glm::vec4 _textColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    std::vector<std::weak_ptr<Character>> _characters;

    std::string _text;
    std::u32string utf8_to_utf32(const std::string& str);
};
} // namespace VkGUI