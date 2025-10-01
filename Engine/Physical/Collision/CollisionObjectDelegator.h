#pragma once
#include <functional>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "CollisionObject.h"

struct CollisionResult
{
    bool intersect = false;
    glm::vec3 axis = {0, 0, 0};
    float depth = 0.0f;
};

class CollisionObjectDelegator : public CollisionObject
{
    friend class CollisionSystem;

public:
    void setCollisionCallback(std::function<void()> func);
    const std::vector<CollisionResult> &collisionResults() const;
    void setSrcLayer(const std::string &layer);
    void setDstLayer(const std::string &layer);

private:
    std::function<void()> _collisionCallback;
    std::vector<CollisionResult> _collisionResults;

    std::string _srcLayer = "";
    std::string _dstLayer = "";

    CollisionObjectDelegator(CollisionObjectType type);
    ~CollisionObjectDelegator();

    void recordCollisionResult(const CollisionResult &result);
    void cleanResult();
};