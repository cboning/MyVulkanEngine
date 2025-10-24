#include "Character.h"
#include "../../Camera/Camera.h"
#include "../../Data.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"

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
    _model.createNewInstance("1", {0, 0.0f});
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

void Character::objectExtraUpdate() { _model.instance("1").object().setPosition(object().position()); }

void Character::onDraw(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t,
                       uint32_t frameIndex) const
{
    uint32_t subpass =
        dynamic_cast<const Vkbase::Pipeline *>(Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Pipeline, pipelineName))->subpass();
    if (subpass != 0)
        return;
    _model.draw(frameIndex, pCommandBuffer, renderPassName, pipelineName, 0);
}

void Character::onUpdateUBO(uint32_t frameIndex) const { _model.instance("1").updateUniformBuffers(frameIndex, camera()); }

void Character::addDescriptorSetsConfig(Vkbase::DescriptorSets &) {}

void Character::writeDescriptorSets(Vkbase::DescriptorSets &) {}

std::vector<vk::DescriptorSetLayout> Character::descriptorSetLayouts() { return _model.descriptorSetLayout("1", "g_buffer"); }

Object &Character::modelObject() { return _model.instance("1").object(); }

const Object &Character::modelObject() const { return _model.instance("1").object(); }
