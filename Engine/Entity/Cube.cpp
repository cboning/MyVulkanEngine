#include "Cube.h"
#include "../../Camera/Camera.h"
#include "../../Data.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"

Cube::Cube(const std::string &name, bool dynamic, const Object &object, bool isOutline) : Entity(name, dynamic, object), _isOutline(isOutline) { init(); }

Cube::~Cube() {}

void Cube::init()
{
    json config = JsonConfigReader::load("config/cube.json");
    for (uint32_t i = 0; i < dynamic_cast<Vkbase::Swapchain *>(Vkbase::ResourceBase::resourceManager().resource(
                                                                   Vkbase::ResourceType::Swapchain, config["descriptorSets"]["sets"][0]["swapchainName"]))
                                 ->imageNames()
                                 .size();
         ++i)
    {
        _ubos.push_back(createResource<Vkbase::Buffer>(name() + "_Cube_UBO_" + std::to_string(i), "Device", sizeof(CubeUniformBufferData),
                                                       vk::BufferUsageFlagBits::eUniformBuffer));
        _ubos.push_back(createResource<Vkbase::Buffer>(name() + "_Cube_Shadow_UBO_" + std::to_string(i), "Device", sizeof(CubeUniformBufferData),
                                                       vk::BufferUsageFlagBits::eUniformBuffer));
    }
    Vkbase::DescriptorSets &descriptorSets = *(createResource<Vkbase::DescriptorSets>(name() + "_Cube", "Device"));
    config["descriptorSets"]["write"][0]["detail"]["bufferInfo"]["bufferName"] = name() + "_Cube_UBO";
    config["descriptorSets"]["write"][1]["detail"]["bufferInfo"]["bufferName"] = name() + "_Cube_Shadow_UBO";
    descriptorSets.addDescriptorSetCreateConfigWithJson(config["descriptorSets"]["sets"]);
    descriptorSets.init();
    descriptorSets.writeSetsWithJson(config["descriptorSets"]["write"]);

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

    createResource<Vkbase::Buffer>("CubeVertex_" + name(), "Device", sizeof(GeometryVertexData) * 24, vk::BufferUsageFlagBits::eVertexBuffer, cubeVertices);

    createResource<Vkbase::Buffer>("CubeIndices_" + name(), "Device", sizeof(uint32_t) * 36, vk::BufferUsageFlagBits::eIndexBuffer, cubeIndices);

    if (_isOutline)
        return;

    if (dynamic())
    {
        CollisionObjectDelegator *pCollisionObject = CollisionSystem::instance().createDynamicObject<CollisionBox>(object());
        pCollisionObject->setEntity(this);
        addCollisionObject(pCollisionObject);
    }
    else
    {
        CollisionObjectDelegator *pCollisionObject = CollisionSystem::instance().createStaticObject<CollisionBox>(object());
        pCollisionObject->setEntity(this);
        addCollisionObject(pCollisionObject);
    }
    collisionObject()->collisionObject<CollisionObject>().updateWithObject(object());
}

void Cube::objectExtraUpdate() {}

void Cube::draw(Vkbase::CommandBuffer *pCommandBuffer, uint32_t frameIndex, const std::string &pipelineName, const std::string &uboName) const
{
    if (_isOutline && pipelineName != "GeometryOutlinePipeline")
        return;
    if (!_isOutline && pipelineName == "GeometryOutlinePipeline")
        return;

    auto *pPipeline = dynamic_cast<Vkbase::Pipeline *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Pipeline, pipelineName));

    auto *pDescriptorSets =
        dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, name() + "_Cube"));

    auto *pVertexBuffer =
        dynamic_cast<Vkbase::Buffer *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, "CubeVertex_" + name()));

    auto *pIndexBuffer =
        dynamic_cast<Vkbase::Buffer *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, "CubeIndices_" + name()));

    if (!pPipeline || !pDescriptorSets || !pVertexBuffer || !pIndexBuffer)
        throw std::runtime_error("[Cube::draw] Missing required resources.");

    pCommandBuffer->bindPipeline(pPipeline);

    pCommandBuffer->bindDescriptorSets(0, {{pDescriptorSets, {uboName, frameIndex}}}, {});

    pCommandBuffer->bindVertexBuffers(0, {pVertexBuffer}, {0});

    pCommandBuffer->bindIndexBuffer(pIndexBuffer, 0, vk::IndexType::eUint32);

    pCommandBuffer->commandBuffer().drawIndexed(36, 1, 0, 0, 0);
}

void Cube::updateUBO(const Camera &camera, uint32_t index, const glm::mat4 &mat, const std::string &uboName) const
{
    CubeUniformBufferData ubo;
    ubo.model = object().matModel();
    ubo.proj = camera.perspective();
    ubo.view = camera.view();
    ubo.lightSpaceMatrix = mat;
    ubo.color = _color;

    dynamic_cast<Vkbase::Buffer *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, name() + "_Cube_" + uboName + "_" + std::to_string(index)))
        ->updateBufferData(&ubo);
}

std::vector<vk::DescriptorSetLayout> Cube::descriptorSetLayouts()
{
    return {dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, name() + "_Cube"))
                ->layout("UBO")};
}
