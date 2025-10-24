#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

struct VertexData
{
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> descriptions;
        descriptions.resize(3);
        uint32_t index = 0;
        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(VertexData, position));

        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(VertexData, texCoord));

        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(VertexData, normal));

        return descriptions;
    }

    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription description;
        description.setBinding(0).setStride(sizeof(VertexData)).setInputRate(vk::VertexInputRate::eVertex);

        return description;
    }
};

struct GeometryVertexData
{
    glm::vec3 position;
    glm::vec3 normal;

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> descriptions;
        descriptions.resize(2);
        uint32_t index = 0;
        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(GeometryVertexData, position));

        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(GeometryVertexData, normal));

        return descriptions;
    }

    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription description;
        description.setBinding(0).setStride(sizeof(GeometryVertexData)).setInputRate(vk::VertexInputRate::eVertex);

        return description;
    }
};

struct ScreenVertex
{
    glm::vec3 position;
    glm::vec2 texCoord;

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> descriptions;
        descriptions.resize(2);
        uint32_t index = 0;
        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(ScreenVertex, position));

        descriptions[index].setBinding(0).setLocation(index++).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(ScreenVertex, texCoord));

        return descriptions;
    }
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription description;
        description.setBinding(0).setStride(sizeof(ScreenVertex)).setInputRate(vk::VertexInputRate::eVertex);

        return description;
    }
};

struct UniformBufferData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};