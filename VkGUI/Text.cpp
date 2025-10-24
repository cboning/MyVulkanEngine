#include "Text.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/CommandBuffer.h"

Text::Text(Font *pFont, const std::string &text, const glm::vec3 &color, const glm::vec2 &pos, float scale)
    : RenderObjectDelegator(pFont->deviceName(), *(Camera *)nullptr, 0, 0), _pFont(pFont), _text(text), _color(color), _pos(pos), _scale(scale)
{
    updateBuffer();
}

Text::Text(Font *pFont) : RenderObjectDelegator(pFont->deviceName(), *(Camera *)nullptr, 0, 0), _pFont(pFont) {}

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

void Text::onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                  uint32_t frameIndex) const
{
    for (uint32_t i = 0; i < _vertexBufferNames.size(); ++i)
        drawCharacter(pCommandBuffer, _text[i], _vertexBufferNames[i], {Font::projectiveSet("MainDescriptorSets")});
}

void Text::drawCharacter(Vkbase::CommandBuffer *pCommandBuffer, const char character, const std::string &vertexBufferName,
                         const vk::ArrayProxy<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> &descriptorSets) const
{
    std::vector<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> descriptorSets_t = {_pFont->set(character)};
    descriptorSets_t.insert(descriptorSets_t.end(), descriptorSets.begin(), descriptorSets.end());

    pCommandBuffer->bindDescriptorSets(0, descriptorSets_t, {});
    pCommandBuffer->bindVertexBuffers(
        0, dynamic_cast<Vkbase::Buffer *>(Vkbase::Buffer::resourceManager().resource(Vkbase::VkResourceType::Buffer, vertexBufferName)), {0});
    pCommandBuffer->commandBuffer().draw(6, 1, 0, 0);
}

void Text::updateBuffer()
{
    for (const std::string &bufferName : _vertexBufferNames)
        Vkbase::Buffer::resourceManager().remove(Vkbase::VkResourceType::Buffer, bufferName);
    _vertexBufferNames.clear();
    _vertexBufferNames.reserve(_text.size());

    glm::vec2 currentPos = _pos;
    currentPos.y *= -1;
    for (std::string::const_iterator c = _text.begin(); c != _text.end(); ++c)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(6);
        auto &charData = _pFont->characters().at(*c);

        glm::vec2 cpos = currentPos + glm::vec2(charData.bearing.x, charData.size.y - charData.bearing.y) * _scale;

        vertices.push_back({glm::vec4(cpos + glm::vec2(0.0f, charData.size.y) * _scale, 0.0f, 0.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(0.0f, 0.0f) * _scale, 0.0f, 1.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(charData.size.x, 0.0f) * _scale, 1.0f, 1.0f)});

        vertices.push_back({glm::vec4(cpos + glm::vec2(0.0f, charData.size.y) * _scale, 0.0f, 0.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(charData.size.x, 0.0f) * _scale, 1.0f, 1.0f)});
        vertices.push_back({glm::vec4(cpos + glm::vec2(charData.size.x, charData.size.y) * _scale, 1.0f, 0.0f)});

        currentPos += glm::vec2(charData.advance >> 6, 0.0f) * _scale;
        _vertexBufferNames.push_back(
            (createResource<Vkbase::Buffer>("", _pFont->deviceName(), 6 * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer, vertices.data()))->name());
    }
}

void Text::onUpdateUBO(uint32_t frameIndex) const {}

void Text::addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) {}

void Text::writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) {}