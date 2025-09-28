#pragma once

class Entity;

class Motion
{
private:
    Entity &_entity;

protected:
    Motion(Entity &entity);
    Entity &entity();

public:
    virtual ~Motion();
    virtual void update(float deltaTime) = 0;
};