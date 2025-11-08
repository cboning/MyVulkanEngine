#include "Cube.h"
#include "../Camera/Camera.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"
#include "../Vkbase/Vkbase.h"

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

void Cube::onDraw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &, const std::string &pipelineName, uint32_t,
                  uint32_t frameIndex) const
{
    std::string setsName;

    if (_isOutline && pipelineName != "GeometryOutlinePipeline")
        return;
    if (!_isOutline && pipelineName == "GeometryOutlinePipeline")
        return;

    auto descriptorSets = Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::DescriptorSets, descriptorSetsName());
    if (auto pDescriptorSets = descriptorSets.lock<Vkbase::DescriptorSets>())
    {
        if (pipelineName == "GeometryPipeline")
        {
            setsName = "UBO";
        }
        else if (pipelineName == "GeometryShadow")
        {
            setsName = "ShadowUBO";
        }
        else
        {
            return;
        }

        _pCubeMesh->draw(commandBuffer, {{descriptorSets, {setsName, frameIndex}}});
    }
    else
        throw std::runtime_error("[Cube::draw] Missing required resources.");
}

void Cube::onUpdateUBO(uint32_t frameIndex) const
{
    for (uint32_t i = 0; i < 2; ++i)
    {
        CubeUniformBufferData ubo;
        ubo.color = _color;
        ubo.lightSpaceMatrix = _lightCamera.perspective() * _lightCamera.view();
        ubo.model = object().matModel();
        ubo.proj = camera().perspective();
        ubo.view = camera().view();
        if (i)
            frameIndex += flightFrameCount() / 2;

        updateUBO(frameIndex, &ubo);
    }
}

void Cube::addDescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets)
{
    if (auto pDescriptor = descriptorSets.lock<Vkbase::DescriptorSets>())
    {
        pDescriptor->addDescriptorSetCreateConfig("UBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, flightFrameCount() / 2);
        pDescriptor->addDescriptorSetCreateConfig("ShadowUBO", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}},
                                                  flightFrameCount() / 2);
    }
}

void Cube::writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &)
{
    writeUBODescriptorSets("UBO", 0, 0, flightFrameCount() / 2);
    writeUBODescriptorSets("ShadowUBO", 0, flightFrameCount() / 2, flightFrameCount());
}

std::vector<vk::DescriptorSetLayout> Cube::descriptorSetLayouts()
{
    if (auto pDescriptorSets =
            Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::DescriptorSets, descriptorSetsName()).lock<Vkbase::DescriptorSets>())
        return {pDescriptorSets->layout("UBO")};
    throw std::runtime_error("Failed found the descriptorSets");
}
