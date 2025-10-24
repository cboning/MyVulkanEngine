#pragma once
#include "../../Data.h"
#include "../../Object/Object.h"
#include "Entity.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>

namespace vk
{
class CommandBuffer;
}

using json = nlohmann::json;

class Camera;

namespace Vkbase
{
class DescriptorSets;
class Buffer;
template <typename T> class Mesh;
} // namespace Vkbase

struct alignas(16) CubeUniformBufferData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpaceMatrix;
    glm::vec3 color;
    float _pad = 0.0f;
};

class Cube : public Entity
{
private:
    inline static Vkbase::Mesh<GeometryVertexData> *_pCubeMesh = nullptr;
    glm::vec3 _color = glm::vec3(0.0f, 0.6f, 0.8f);
    std::vector<Vkbase::Buffer *> _ubos;
    bool _isOutline;
    const Camera &_lightCamera;

    void entityInit() override;
    void objectExtraUpdate() override;

public:
    Cube(const std::string &name, const std::string &deviceName, const Camera &camera, const Camera &lightCamera, bool dynamic = true, const Object &object = {}, bool isOutline = false);
    ~Cube();
    void onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex, uint32_t frameIndex) const override;
    void onUpdateUBO(uint32_t frameIndex) const override;
    void addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets) override;
    void writeDescriptorSets(Vkbase::DescriptorSets &descriptorSets) override;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() override;
}; 