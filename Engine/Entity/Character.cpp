#include "Character.h"
#include "../Camera/Camera.h"
#include "../Data.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"
#include "../Vkbase/Vkbase.h"

Character::Character(const std::string &name, const std::string &deviceName, const Camera &camera, const Camera &lightCamera, Modelbase::Model &model,
                     bool dynamic, const Object &object)
    : Entity(deviceName, camera, 0, 0, name, dynamic, object), _model(model)
{
    entityInit();
    delegatorInit();
}

Character::~Character() {}

void Character::entityInit()
{
    for (uint32_t i = 0;; ++i)
    {
        _instanceName = std::to_string(i);
        if (_model.instanceIndex(_instanceName) == -1)
            break;
    }

    _model.createNewInstance(_instanceName, {0, 0.0f});
    if (dynamic())
    {
        CollisionObjectDelegator *pCollisionObject =
            CollisionSystem::instance().createDynamicObject<CollisionCapsule>(object().position(), 1.0f, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
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

void Character::objectExtraUpdate() { _model.instance(_instanceName).object().setPosition(object().position()); }

void Character::onDraw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName,
                       uint32_t, uint32_t frameIndex) const
{
    if (auto pPipeline = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Pipeline, pipelineName).lock<Vkbase::Pipeline>())
    {
        uint32_t subpass = pPipeline->subpass();
        if (subpass != 0)
            return;
        _model.draw(frameIndex, commandBuffer, renderPassName, pipelineName, 0);
    }
    else
        throw std::runtime_error("Failed to get the pipeline.");
}

void Character::onUpdateUBO(uint32_t frameIndex) const { _model.instance(_instanceName).updateUniformBuffers(frameIndex, camera()); }

void Character::addDescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &) {}

void Character::writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &) {}

std::vector<vk::DescriptorSetLayout> Character::descriptorSetLayouts() { return _model.descriptorSetLayout(_instanceName, "g_buffer"); }

Object &Character::modelObject() { return _model.instance(_instanceName).object(); }

const Object &Character::modelObject() const { return _model.instance(_instanceName).object(); }
