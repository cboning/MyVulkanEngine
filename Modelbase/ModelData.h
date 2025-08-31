#pragma once

#include <GLFW/glfw3.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <vector>

#define MAX_BONE_INFLUENCE 4

namespace ModelData
{
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 color;
    int boneIds[MAX_BONE_INFLUENCE];
    float weights[MAX_BONE_INFLUENCE];
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription description;
        description.setBinding(0).setStride(sizeof(Vertex)).setInputRate(vk::VertexInputRate::eVertex);

        return description;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> descriptions;
        descriptions.resize(6);

        descriptions[0].setBinding(0).setFormat(vk::Format::eR32G32B32Sfloat).setLocation(0).setOffset(offsetof(Vertex, pos));

        descriptions[1].setBinding(0).setFormat(vk::Format::eR32G32B32Sfloat).setLocation(1).setOffset(offsetof(Vertex, normal));

        descriptions[2].setBinding(0).setFormat(vk::Format::eR32G32Sfloat).setLocation(2).setOffset(offsetof(Vertex, texCoord));

        descriptions[3].setBinding(0).setFormat(vk::Format::eR32G32B32Sfloat).setLocation(3).setOffset(offsetof(Vertex, color));

        descriptions[4].setBinding(0).setFormat(vk::Format::eR32G32B32A32Sint).setLocation(4).setOffset(offsetof(Vertex, boneIds));

        descriptions[5].setBinding(0).setFormat(vk::Format::eR32G32B32A32Sfloat).setLocation(5).setOffset(offsetof(Vertex, weights));

        return descriptions;
    }
};

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

struct BoneInfo
{
    int id;
    glm::mat4 offset;
};

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat rotation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};
} // namespace ModelData
