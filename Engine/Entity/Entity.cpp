#include "Entity.h"
#include "../../Camera/Camera.h"
#include "../Physical/Collision/CollisionObjectDelegator.h"
#include "../Physical/Collision/CollisionSystem.h"
#include "../Physical/Motion/Motion.h"
#include <iostream>

Entity::Entity(const std::string &name) : _name(name)
{
    if (_pEntities.count(name))
        throw std::runtime_error("The Entity name " + name + " already exist.");
    _pEntities.insert({name, (std::unique_ptr<Entity, Deleter>(this))});

    _pCollisionObjectDelegator = CollisionSystem::instance().createDynamicObject(CollisionObjectType::Box);
}

Entity::~Entity()
{
    CollisionSystem::instance().destoryDynamicObject(_pCollisionObjectDelegator);
    for (auto &motion : _pMotions)
        delete motion.second;
}

CollisionObjectDelegator &Entity::collisionObject() { return *_pCollisionObjectDelegator; }

std::unordered_map<std::string, Motion *> &Entity::motions() { return _pMotions; }

const std::string &Entity::name() const { return _name; }

Object &Entity::object() { return _object; }

const Object &Entity::object() const { return _object; }

const CollisionObjectDelegator &Entity::collisionObject() const { return *_pCollisionObjectDelegator; }

void Entity::updateCollisionObject() { _pCollisionObjectDelegator->updateWithObject(_object); }

void Entity::updateVelocity(float deltaTime) { _velocity += deltaTime * _acceleration; }

void Entity::updatePosition(float deltaTime) { _object.setPosition(deltaTime * _velocity + _object.position()); }

void Entity::update(float deltaTime)
{
    updatePosition(deltaTime);
    updateVelocity(deltaTime);
}

void Entity::updateCollisionObjects()
{
    for (auto &pEntity : _pEntities)
        pEntity.second->updateCollisionObject();
}

void Entity::updateAll(float deltaTime)
{
    for (auto &pEntity : _pEntities)
    {
        pEntity.second->update(deltaTime);
        pEntity.second->_acceleration = glm::vec3(0.0f);
        for (auto motion : pEntity.second->motions())
            motion.second->update(deltaTime);
    }
}

glm::vec3 &Entity::acceleration() { return _acceleration; }

const glm::vec3 &Entity::acceleration() const { return _acceleration; }

glm::vec3 &Entity::velocity() { return _velocity; }

const glm::vec3 &Entity::velocity() const { return _velocity; }

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

void Entity::drawEntities(const vk::CommandBuffer &commandBuffer, const Camera &camera, uint32_t index)
{
    for (auto &entity : _pEntities)
    {
        entity.second->updateUBO(camera, index);
        entity.second->draw(commandBuffer, index);
    }
}
