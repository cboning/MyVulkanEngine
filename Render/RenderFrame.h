#pragma once
#include "../Vkbase/RenderObjectDelegator.h"
#include <glm/glm.hpp>
#include <string>

namespace Vkbase
{
class DescriptorSets;
class CommandBuffer;
class Buffer;
} // namespace Vkbase

class RenderFrame : public Vkbase::RenderObjectDelegator
{
public:
    struct ScreenVertexData
    {
        glm::vec3 position;
        glm::vec2 texCoord;

        static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
        {
            std::vector<vk::VertexInputAttributeDescription> descriptions;
            descriptions.resize(2);
            uint32_t index = 0;
            descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(ScreenVertexData, position));

            descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(ScreenVertexData, texCoord));

            return descriptions;
        }

        static vk::VertexInputBindingDescription bindingDescription()
        {
            vk::VertexInputBindingDescription description;
            description.setBinding(0).setStride(sizeof(ScreenVertexData)).setInputRate(vk::VertexInputRate::eVertex);

            return description;
        }
    };

    RenderFrame(const std::string &deviceName,
                const std::vector<std::vector<std::pair<std::string, std::pair<std::string, uint32_t>>>> &descriptorSets);

    RenderFrame(RenderFrame&& other) noexcept = default;

private:
    Vkbase::Buffer *_pFrameVerticesBuffer;
    std::vector<std::vector<std::pair<std::string, std::pair<std::string, uint32_t>>>> _descriptorSets;
    void onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex, uint32_t frameIndex) const override;
    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) override;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) override;
};