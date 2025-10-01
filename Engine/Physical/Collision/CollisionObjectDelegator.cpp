#include "CollisionObjectDelegator.h"

CollisionObjectDelegator::CollisionObjectDelegator(CollisionObjectType type) : CollisionObject(type) {}

void CollisionObjectDelegator::setCollisionCallback(std::function<void()> func) { _collisionCallback = func; }

const std::vector<CollisionResult> &CollisionObjectDelegator::collisionResults() const { return _collisionResults; }

void CollisionObjectDelegator::setSrcLayer(const std::string &layer) { _srcLayer = layer; }

void CollisionObjectDelegator::setDstLayer(const std::string &layer) { _dstLayer = layer; }

CollisionObjectDelegator::~CollisionObjectDelegator() {}

void CollisionObjectDelegator::recordCollisionResult(const CollisionResult &result)
{
    _collisionResults.push_back(result);
    if (result.intersect && _collisionCallback)
        _collisionCallback();
}

void CollisionObjectDelegator::cleanResult() { _collisionResults.clear(); }
