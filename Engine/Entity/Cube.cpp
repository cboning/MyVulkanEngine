#include "Cube.h"
#include "../../Camera/Camera.h"
#include "../../Data.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"
#include "../Physical/Motion/Gravity.h"
#include "../Physical/Motion/Collision.h"

Cube::Cube(const std::string &name, bool enableGravity) : Entity(name)
{
    if (enableGravity){
        motions().insert((Motion *)(new Gravity(*this)));
        motions().insert((Motion *)(new Collision(*this)));
    }
    init();
}

Cube::~Cube()
{
    for (uint32_t i = 0; i < 5; ++i)
        Vkbase::ResourceBase::resourceManager().remove(Vkbase::ResourceType::Buffer, name() + "_Cube_UBO_" + std::to_string(i));
    Vkbase::ResourceBase::resourceManager().remove(Vkbase::ResourceType::DescriptorSets, name() + "_Cube");

    Vkbase::ResourceBase::resourceManager().remove(Vkbase::ResourceType::Buffer, "CubeVertex_" + name());
    Vkbase::ResourceBase::resourceManager().remove(Vkbase::ResourceType::Buffer, "CubeIndices_" + name());
}

void Cube::init()
{
    collisionObject().setPositionInBoundBox(glm::vec3(0.5f));
    for (uint32_t i = 0; i < 5; ++i)
        Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>(name() + "_Cube_UBO_" + std::to_string(i), "Device", sizeof(CubeUniformBufferData),
                                                                       vk::BufferUsageFlagBits::eUniformBuffer);
    Vkbase::DescriptorSets &descriptorSets = *(Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>(name() + "_Cube", "Device"));
    descriptorSets.addDescriptorSetCreateConfigWithJson(JsonConfigReader::load("./config/cube_" + name() + ".json")["descriptorSets"]["sets"]);
    descriptorSets.init();
    descriptorSets.writeSetsWithJson(JsonConfigReader::load("config/cube_" + name() + ".json")["descriptorSets"]["write"]);

    GeometryVertexData cubeVertices[] = {{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},   {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                         {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},     {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

                                         {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
                                         {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},  {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

                                         {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
                                         {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},

                                         {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                         {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},

                                         {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
                                         {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},   {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},

                                         {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                         {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}};

    uint32_t cubeIndices[] = {0,  1,  2,  2,  3,  0,

                              4,  5,  6,  6,  7,  4,

                              8,  9,  10, 10, 11, 8,

                              12, 13, 14, 14, 15, 12,

                              16, 17, 18, 18, 19, 16,

                              20, 21, 22, 22, 23, 20};

    Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>("CubeVertex_" + name(), "Device", sizeof(GeometryVertexData) * 24,
                                                                   vk::BufferUsageFlagBits::eVertexBuffer, cubeVertices);

    Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>("CubeIndices_" + name(), "Device", sizeof(uint32_t) * 36,
                                                                   vk::BufferUsageFlagBits::eIndexBuffer, cubeIndices);

    collisionObject().updateWithObject(object());
}

void Cube::draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex) const
{
    Vkbase::Pipeline &pipeline =
        *dynamic_cast<Vkbase::Pipeline *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Pipeline, "GeometryPipeline"));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline());
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0,
        {dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, name() + "_Cube"))
             ->sets("UBO")[frameIndex]},
        {});
    commandBuffer.bindVertexBuffers(
        0, {dynamic_cast<Vkbase::Buffer *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, "CubeVertex_" + name()))->buffer()},
        {0});
    commandBuffer.bindIndexBuffer(
        {dynamic_cast<Vkbase::Buffer *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, "CubeIndices_" + name()))->buffer()}, 0,
        vk::IndexType::eUint32);
    commandBuffer.drawIndexed(36, 1, 0, 0, 0);
}

void Cube::updateUBO(const Camera &_camera, uint32_t index) const
{
    CubeUniformBufferData ubo;
    ubo.model = object().matModel();
    ubo.proj = _camera.perspective();
    ubo.view = _camera.view();
    ubo.color = _color;

    dynamic_cast<Vkbase::Buffer *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, name() + "_Cube_UBO_" + std::to_string(index)))
        ->updateBufferData(&ubo);
}

bool Cube::checkCollisionWithObject(const CollisionObject &target)
{
    CollisionResult result = collisionObject().performCollisionDetection(target);
    collisionResults().push_back(result);

    // if (result.intersect)
    //     _color = glm::vec3(1.0f, 0.0f, 0.0f);
    // else
    //     _color = glm::vec3(0.0f, 1.0f, 0.0f);

    return result.intersect;
}

bool Cube::checkCollisionWithObject(Cube &target) { return checkCollisionWithObject(target.collisionObject()); }

std::vector<vk::DescriptorSetLayout> Cube::descriptorSetLayouts()
{
    return {dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, name() + "_Cube"))
                ->layout("UBO")};
}
