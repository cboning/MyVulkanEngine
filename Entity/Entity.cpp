#include "Entity.h"

Entity::Entity(const std::string &name)
    : _name(name)
{
    if (_entities.count(name))
        throw std::runtime_error("The Entity name " + name + " already exist.");
    _entities.insert({name, this});
}

Entity::~Entity()
{
    _entities.erase(_name);
}

BoxCollisionObject &Entity::collisionObject()
{
    return _collisionObject;
}

const std::string &Entity::name() const
{
    return _name;
}

Object &Entity::object()
{
    return _object;
}

const Object &Entity::object() const
{
    return _object;
}

Entity &Entity::entity(const std::string &name)
{
    return *_entities.at(name);
}