#include "ModelEntity.h"
#include "../../Camera/Camera.h"
#include "../../Data.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionCapsule.h"
#include "../Physical/Collision/CollisionTriangle.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"

ModelEntity::ModelEntity(const std::string &name, bool dynamic, const Object &object, const json &config)
    : Entity(name, dynamic, object),
      _model("Device", dynamic_cast<Vkbase::Sampler *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler(),
             config)
{
    init();
}

ModelEntity::~ModelEntity() {}

void ModelEntity::init()
{
    _model.createNewInstance("1", {0, 0.0f});
    if (dynamic())
    {
        for (const auto &mesh : _model.meshes())
            for (uint32_t i = 0; i < mesh.indices().size(); i += 3)
            {
                CollisionObjectDelegator *pCollisionObject = CollisionSystem::instance().createDynamicObject<CollisionTriangle>(
                    object().position(),
                    glm::mat3(mesh.vertices()[mesh.indices()[i]].pos, mesh.vertices()[mesh.indices()[i + 1]].pos, mesh.vertices()[mesh.indices()[i + 2]].pos));
                pCollisionObject->setEntity(this);
                addCollisionObject(pCollisionObject);
            }
    }
    else
    {
        for (const auto &mesh : _model.meshes())
            for (uint32_t i = 0; i < mesh.indices().size(); i += 3)
            {
                CollisionObjectDelegator *pCollisionObject = CollisionSystem::instance().createStaticObject<CollisionTriangle>(
                    object().position(),
                    glm::mat3(mesh.vertices()[mesh.indices()[i]].pos, mesh.vertices()[mesh.indices()[i + 1]].pos, mesh.vertices()[mesh.indices()[i + 2]].pos));
                pCollisionObject->setEntity(this);
                addCollisionObject(pCollisionObject);
            }
    }
    collisionObject()->collisionObject<CollisionObject>().updateWithObject(object());
}

void ModelEntity::objectExtraUpdate() { _model.instance("1").object().setPosition(object().position()); }

void ModelEntity::draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex, const std::string &pipelineName, const std::string &uboName) const
{
    _model.draw(frameIndex, commandBuffer, 0);
}

void ModelEntity::updateUBO(const Camera &camera, uint32_t index, const glm::mat4 &mat, const std::string &uboName) const
{
    const Modelbase::ModelInstance &instance = _model.instance("1");
    instance.updateUniformBuffers(index, camera);
}

std::vector<vk::DescriptorSetLayout> ModelEntity::descriptorSetLayouts() { return _model.descriptorSetLayout("1", "g_buffer"); }

Object &ModelEntity::modelObject() { return _model.instance("1").object(); }

const Object &ModelEntity::modelObject() const { return _model.instance("1").object(); }
