#pragma once
#include <functional>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Entity;

class CollisionObject;

class CollisionObjectDelegator;

struct CollisionResult
{
    bool intersect = false;
    glm::vec3 axis = {0, 0, 0};
    float depth = 0.0f;
    const CollisionObjectDelegator *pTarget;
};

class CollisionObjectDelegator
{
    friend class CollisionSystem;

public:
    CollisionObjectDelegator(const CollisionObjectDelegator &) = delete;
    CollisionObjectDelegator &operator=(const CollisionObjectDelegator &) = delete;

    void setCollisionCallback(std::function<void()> func);
    const std::vector<CollisionResult> &collisionResults() const;
    void setSrcLayer(const std::string &layer);
    void setDstLayer(const std::string &layer);

    void setEntity(Entity *pEntity);
    template <typename T> const T &collisionObject() const { return *dynamic_cast<T *>(_pCollisionObject); }
    template <typename T> T &collisionObject() { return *dynamic_cast<T *>(_pCollisionObject); }

    const Entity *entity() const;

private:
    std::function<void()> _collisionCallback;
    std::vector<CollisionResult> _collisionResults;

    std::string _srcLayer = "";
    std::string _dstLayer = "";

    Entity *_pEntity = nullptr;

    CollisionObject *_pCollisionObject;

    CollisionObjectDelegator(CollisionObject *pCollisionObject);
    ~CollisionObjectDelegator();

    void recordCollisionResult(const CollisionResult &result);
    void cleanResult();
};