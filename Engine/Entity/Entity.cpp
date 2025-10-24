#include "Entity.h"
#include "../../Camera/Camera.h"
#include "../Physical/Collision/CollisionBox.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"
#include "Motion/Motion.h"
#include <iostream>

Entity::Entity(const std::string &deviceName, const Camera &camera, uint32_t flightFrameCount, vk::DeviceSize uboSize, const std::string &name, bool dynamic,
               const Object &object)
    : RenderObjectDelegator(deviceName, camera, flightFrameCount, uboSize), _name(name), _object(object), _dynamic(dynamic)
{
    if (_pEntities.count(name))
        throw std::runtime_error("The Entity name " + name + " already exist.");
    _pEntities.insert({name, (std::unique_ptr<Entity, Deleter>(this))});
}

Entity::~Entity()
{
    for (CollisionObjectDelegator *pCollisionObjectDelegator : _pCollisionObjectDelegators)
        CollisionSystem::instance().destroyDynamicObject(pCollisionObjectDelegator);
    for (auto &motion : _pMotions)
        delete motion.second;
}

CollisionObjectDelegator *Entity::collisionObject() { return _pCollisionObjectDelegators[0]; }

CollisionObjectDelegator *Entity::collisionObject(uint32_t index) { return _pCollisionObjectDelegators[index]; }

uint32_t Entity::collisionObjectDelegatorsCount() const { return _pCollisionObjectDelegators.size(); }

std::unordered_map<std::string, Motion *> &Entity::motions() { return _pMotions; }

void Entity::addCollisionObject(CollisionObjectDelegator *pCollisionObjectDelegator) { _pCollisionObjectDelegators.push_back(pCollisionObjectDelegator); }

const std::string &Entity::name() const { return _name; }

bool Entity::dynamic() { return _dynamic; }

Object &Entity::object() { return _object; }

const Object &Entity::object() const { return _object; }

const CollisionObjectDelegator *Entity::collisionObject() const { return _pCollisionObjectDelegators[0]; }

void Entity::updateCollisionObject()
{
    for (auto pCollisionObjectDelegator : _pCollisionObjectDelegators)
        pCollisionObjectDelegator->collisionObject<CollisionObject>().updateWithObject(object());
}

void Entity::updateVelocity(float deltaTime) { _velocity += deltaTime * _acceleration; }

void Entity::updatePosition(float deltaTime) { _object.setPosition(deltaTime * _velocity + _object.position()); }

void Entity::updatePhysicalState(float deltaTime)
{
    updatePosition(deltaTime);
    objectExtraUpdate();
    updateVelocity(deltaTime);
    _tempVelocity = _velocity;
}

void Entity::updateCollisionObjects()
{
    for (auto &pEntity : _pEntities)
        pEntity.second->updateCollisionObject();
}

void Entity::updateAll(float deltaTime)
{
    for (auto &pEntity : _pEntities)
        pEntity.second->updatePhysicalState(deltaTime);

    for (auto &pEntity : _pEntities)
    {
        pEntity.second->_acceleration = glm::vec3(0.0f);
        for (auto motion : pEntity.second->motions())
            motion.second->update(deltaTime);
    }
}

glm::vec3 &Entity::acceleration() { return _acceleration; }

const glm::vec3 &Entity::acceleration() const { return _acceleration; }

glm::vec3 &Entity::velocity() { return _velocity; }

const glm::vec3 &Entity::velocity() const { return _velocity; }

glm::vec3 &Entity::tempVelocity() { return _tempVelocity; }

const glm::vec3 &Entity::tempVelocity() const { return _tempVelocity; }

Motion *Entity::addMotion(const std::string &name, Motion *pMotion)
{
    if (_pMotions.count(name))
    {
        std::cout << "Failed to add Motion. Reason: The name of motion " << name << " already exist." << std::endl;
        delete pMotion;
        return nullptr;
    }

    _pMotions[name] = pMotion;
    pMotion->setEntity(*this);
    return pMotion;
}

Motion *Entity::motion(const std::string &name)
{
    if (!_pMotions.count(name))
        return nullptr;
    return _pMotions.at(name);
}

void Entity::eraseMotion(const std::string &name)
{
    if (!_pMotions.count(name))
    {
        std::cout << "Failed to erase Motion. Reason: The name of motion " << name << " already exist." << std::endl;
        return;
    }

    delete _pMotions.extract(name).mapped();
}

void Entity::drawEntities(Vkbase::CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                          uint32_t frameIndex)
{
    for (auto &entity : _pEntities)
        entity.second->draw(pCommandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);
}

void Entity::updateEntities(uint32_t frameIndex)
{
    for (auto &entity : _pEntities)
        entity.second->update(frameIndex);
}
