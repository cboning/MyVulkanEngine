#pragma once
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Pipeline.h"
#include "Font.h"
#include <vulkan/vulkan.hpp>

class Text
{
  public:
    Text(const Font &font);
    Text(const Font &font, const std::string &text, const glm::vec3 &color, const glm::vec2 &pos, float scale);
    ~Text();
    void setText(const std::string &text);
    void setColor(const glm::vec3 &color);
    void setPos(const glm::vec2 &pos);
    void setScale(float scale);
    void draw(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const vk::ArrayProxy<const vk::DescriptorSet> &descriptorSets);

    struct Vertex
    {
        glm::vec4 posTex;

        static vk::VertexInputBindingDescription bindingDescription()
        {
            vk::VertexInputBindingDescription description;
            description.setBinding(0).setStride(sizeof(Vertex)).setInputRate(vk::VertexInputRate::eVertex);

            return description;
        }

        static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
        {
            std::vector<vk::VertexInputAttributeDescription> descriptions;
            descriptions.resize(1);

            descriptions[0].setBinding(0).setFormat(vk::Format::eR32G32B32A32Sfloat).setLocation(0).setOffset(offsetof(Vertex, posTex));

            return descriptions;
        }
    };

  private:
    const Font &_font;
    std::vector<std::string> _vertexBufferNames;
    std::string _text;
    glm::vec3 _color;
    glm::vec2 _pos;
    float _scale;

    void drawCharacter(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const char character, const std::string &vertexBufferName,
                       const vk::ArrayProxy<const vk::DescriptorSet> &descriptorSets);
    void updateBuffer();
};