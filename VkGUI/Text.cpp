#include "Text.h"
#include "../Vkbase/Buffer.h"

Text::Text(const Font &font, const std::string &text, const glm::vec3 &color, const glm::vec2 &pos, float scale)
    : _font(font), _text(text), _color(color), _pos(pos), _scale(scale)
{
    updateBuffer();
}

Text::Text(const Font &font) : _font(font) {}

Text::~Text()
{
    for (const std::string &bufferName : _vertexBufferNames)
        Vkbase::ResourceManager().remove(Vkbase::ResourceType::Buffer, bufferName);
    _vertexBufferNames.clear();
}

void Text::setText(const std::string &text)
{
    _text = text;
    updateBuffer();
}

void Text::setColor(const glm::vec3 &color) { _color = color; }

void Text::setPos(const glm::vec2 &pos)
{
    _pos = pos;
    updateBuffer();
}

void Text::setScale(float scale)
{
    _scale = scale;
    updateBuffer();
}

void Text::draw(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const vk::ArrayProxy<const vk::DescriptorSet> &descriptorSets)
{
    for (uint32_t i = 0; i < _vertexBufferNames.size(); ++i)
        drawCharacter(commandBuffer, pipeline, _text[i], _vertexBufferNames[i], descriptorSets);
}

void Text::drawCharacter(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const char character, const std::string &vertexBufferName,
                         const vk::ArrayProxy<const vk::DescriptorSet> &descriptorSets)
{
    std::vector<vk::DescriptorSet> descriptorSets_t = {_font.set(character)};
    descriptorSets_t.insert(descriptorSets_t.end(), descriptorSets.begin(), descriptorSets.end());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0, descriptorSets_t, {});
    commandBuffer.bindVertexBuffers(
        0, dynamic_cast<const Vkbase::Buffer *>(Vkbase::Buffer::resourceManager().resource(Vkbase::ResourceType::Buffer, vertexBufferName))->buffer(), {0});
    commandBuffer.draw(6, 1, 0, 0);
}

void Text::updateBuffer()
{
    for (const std::string &bufferName : _vertexBufferNames)
        Vkbase::Buffer::resourceManager().remove(Vkbase::ResourceType::Buffer, bufferName);
    _vertexBufferNames.clear();
    _vertexBufferNames.reserve(_text.size());

    glm::vec2 currentPos = _pos;
    currentPos.y *= -1;
    for (std::string::const_iterator c = _text.begin(); c != _text.end(); ++c)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(6);
        auto &charData = _font.characters().at(*c);

        glm::vec2 cpos = currentPos + glm::vec2(charData.bearing.x, charData.size.y - charData.bearing.y) * _scale;

        vertices.push_back({glm::vec4(cpos + glm::vec2(0.0f, charData.size.y) * _scale, 0.0f, 0.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(0.0f, 0.0f) * _scale, 0.0f, 1.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(charData.size.x, 0.0f) * _scale, 1.0f, 1.0f)});

        vertices.push_back({glm::vec4(cpos + glm::vec2(0.0f, charData.size.y) * _scale, 0.0f, 0.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(charData.size.x, 0.0f) * _scale, 1.0f, 1.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(charData.size.x, charData.size.y) * _scale, 1.0f, 0.0f)});

        currentPos += glm::vec2(charData.advance >> 6, 0.0f) * _scale;
        _vertexBufferNames.push_back(
            (new Vkbase::Buffer("", _font.deviceName(), 6 * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer, vertices.data()))->name());
    }
}