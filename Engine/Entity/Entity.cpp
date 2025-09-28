#include "Entity.h"
#include "../Physical/Motion/Motion.h"

Entity::Entity(const std::string &name) : _name(name)
{
    if (_pEntities.count(name))
        throw std::runtime_error("The Entity name " + name + " already exist.");
    _pEntities.insert({name, this});
}

Entity::~Entity()
{
    _pEntities.erase(_name);
    for (Motion *motion : _pMotions)
        delete motion;
}

BoxCollisionObject &Entity::collisionObject() { return _collisionObject; }

std::unordered_set<Motion *> &Entity::motions() { return _pMotions; }

std::vector<CollisionResult> &Entity::collisionResults() { return _collisionResults; }

const std::string &Entity::name() const { return _name; }

Object &Entity::object() { return _object; }

const Object &Entity::object() const { return _object; }

void Entity::updateCollisionObject() { _collisionObject.updateWithObject(_object); }

void Entity::updateVelocity(float deltaTime) { _velocity += deltaTime * _acceleration; }

void Entity::updatePosition(float deltaTime) { _object.setPosition(deltaTime * _velocity + _object.position()); }

void Entity::update(float deltaTime)
{
    updatePosition(deltaTime);
    updateVelocity(deltaTime);
}

void Entity::updateCollisionObjects()
{
    for (auto pEntity : _pEntities)
        pEntity.second->updateCollisionObject();
}

void Entity::updateAll(float deltaTime)
{
    for (auto pEntity : _pEntities)
    {
        pEntity.second->update(deltaTime);
        pEntity.second->_acceleration = glm::vec3(0.0f);
        for (auto motion : pEntity.second->motions())
            motion->update(deltaTime);
    }
}

glm::vec3 &Entity::acceleration() { return _acceleration; }

const glm::vec3 &Entity::acceleration() const { return _acceleration; }

void Entity::cleanCollisionResults() { _collisionResults.clear(); }

const std::vector<CollisionResult> &Entity::collisionResults() const { return _collisionResults; }

void Entity::cleanAllCollisionResults()
{
    for (auto pEntity : _pEntities)
        pEntity.second->cleanCollisionResults();
}

glm::vec3 &Entity::velocity() { return _velocity; }

const glm::vec3 &Entity::velocity() const { return _velocity; }
