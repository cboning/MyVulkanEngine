#pragma once

class Entity;

class Motion
{
    friend Entity;
private:
    Entity *_pEntity = nullptr;

    void setEntity(Entity &entity);

protected:
    Motion();
    Entity &entity();

public:
    virtual ~Motion();
    virtual void update(float deltaTime) = 0;
};