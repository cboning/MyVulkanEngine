#include "Cube.h"
#include "../../Camera/Camera.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"

Cube::Cube(const std::string &name, const std::string &deviceName, const Camera &camera, const Camera &lightCamera, bool dynamic, const Object &object,
           bool isOutline)
    : Entity(deviceName, camera, MAX_FLIGHT_COUNT * 2, sizeof(CubeUniformBufferData), name, dynamic, object), _isOutline(isOutline), _lightCamera(lightCamera)
{
    entityInit();
    delegatorInit();
}

Cube::~Cube() {}

void Cube::entityInit()
{
    if (!_pCubeMesh)
    {
        std::vector<GeometryVertexData> cubeVertices = {{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},   {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
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

        std::vector<uint16_t> cubeIndices = {0,  1,  2,  2,  3,  0,

                                             4,  5,  6,  6,  7,  4,

                                             8,  9,  10, 10, 11, 8,

                                             12, 13, 14, 14, 15, 12,

                                             16, 17, 18, 18, 19, 16,

                                             20, 21, 22, 22, 23, 20};

        _pCubeMesh = new Vkbase::Mesh<GeometryVertexData>("Cube", deviceName(), cubeVertices, cubeIndices, {}, "");
    }

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

void Cube::onDraw(Vkbase::CommandBuffer *pCommandBuffer, uint32_t frameIndex, const std::vector<std::any> &args) const
{
    std::string pipelineName = std::any_cast<std::string>(args[0]);
    std::string setsName = std::any_cast<std::string>(args[1]);

    if (_isOutline && pipelineName != "GeometryOutlinePipeline")
        return;
    if (!_isOutline && pipelineName == "GeometryOutlinePipeline")
        return;

    auto *pPipeline = dynamic_cast<Vkbase::Pipeline *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Pipeline, pipelineName));

    auto *pDescriptorSets =
        dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName()));

    if (!pPipeline || !pDescriptorSets)
        throw std::runtime_error("[Cube::draw] Missing required resources.");

    _pCubeMesh->draw(pCommandBuffer, *pPipeline, {{pDescriptorSets, {setsName, frameIndex}}});
}

void Cube::onUpdateUBO(uint32_t frameIndex, const std::vector<std::any> &args) const
{
    CubeUniformBufferData ubo;
    ubo.color = _color;
    ubo.lightSpaceMatrix = _lightCamera.perspective() * _lightCamera.view();
    ubo.model = object().matModel();
    ubo.proj = camera().perspective();
    ubo.view = camera().view();
    if (std::any_cast<bool>(args[2]))
        frameIndex += flightFrameCount() / 2;

    updateUBO(frameIndex, &ubo);
}

void Cube::addDescriptorSetsConfig(Vkbase::DescriptorSets &descriptorSets)
{
    descriptorSets.addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, flightFrameCount() / 2);
    descriptorSets.addDescriptorSetCreateConfig("ShadowUBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, flightFrameCount() / 2);
}

void Cube::writeDescriptorSets(Vkbase::DescriptorSets &)
{
    writeUBODescriptorSets("UBO", 0, 0, flightFrameCount() / 2);
    writeUBODescriptorSets("ShadowUBO", 0, flightFrameCount() / 2, flightFrameCount());
}

std::vector<vk::DescriptorSetLayout> Cube::descriptorSetLayouts()
{
    return {dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName()))
                ->layout("UBO")};
}
