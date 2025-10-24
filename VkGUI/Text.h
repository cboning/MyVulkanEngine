#pragma once
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Pipeline.h"
#include "../Vkbase/RenderObjectDelegator.h"
#include "Font.h"
#include <vulkan/vulkan.hpp>

namespace Vkbase
{
class DescriptorSets;
}

class Text : public Vkbase::RenderObjectDelegator
{
    friend class Font;

public:
    ~Text() = default;
    void setText(const std::string &text);
    void setColor(const glm::vec3 &color);
    void setPos(const glm::vec2 &pos);
    void setScale(float scale);

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
    Text(Font *font);
    Text(Font *font, const std::string &text, const glm::vec3 &color, const glm::vec2 &pos, float scale);
    Font *_pFont;
    std::vector<std::string> _vertexBufferNames;
    std::string _text;
    glm::vec3 _color;
    glm::vec2 _pos;
    float _scale;

    void drawCharacter(Vkbase::CommandBuffer *pCommandBuffer, const char character, const std::string &vertexBufferName,
                       const vk::ArrayProxy<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> &descriptorSets) const;
    void updateBuffer();
    void onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                uint32_t frameIndex) const override;
    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) override;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) override;
};