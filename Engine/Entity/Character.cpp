#include "Character.h"
#include "../../Camera/Camera.h"
#include "../../Data.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"

Character::Character(const std::string &name, bool dynamic, const Object &object, const json &config)
    : Entity(name, dynamic, object),
      _model("Device", dynamic_cast<Vkbase::Sampler *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler(),
             config)
{
    init();
}

Character::~Character() {}

void Character::init()
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

void Character::draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex, const std::string &pipelineName, const std::string &uboName) const
{
    _model.draw(frameIndex, commandBuffer, 0);
}

void Character::updateUBO(const Camera &camera, uint32_t index, const glm::mat4 &mat, const std::string &uboName) const
{
    const Modelbase::ModelInstance &instance = _model.instance("1");
    instance.updateUniformBuffers(index, camera);
}

std::vector<vk::DescriptorSetLayout> Character::descriptorSetLayouts() { return _model.descriptorSetLayout("1", "g_buffer"); }

Object &Character::modelObject() { return _model.instance("1").object(); }

const Object &Character::modelObject() const { return _model.instance("1").object(); }
