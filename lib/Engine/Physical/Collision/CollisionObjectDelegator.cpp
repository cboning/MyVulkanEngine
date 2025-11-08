#include "CollisionObjectDelegator.h"
#include "CollisionSystem.h"

CollisionObjectDelegator::CollisionObjectDelegator(CollisionObject *pCollisionObject) : _pCollisionObject(pCollisionObject) {}

void CollisionObjectDelegator::setCollisionCallback(std::function<void()> func) { _collisionCallback = func; }

const std::vector<CollisionResult> &CollisionObjectDelegator::collisionResults() const { return _collisionResults; }

void CollisionObjectDelegator::setSrcLayer(const std::string &layer) { _srcLayer = layer; }

void CollisionObjectDelegator::setDstLayer(const std::string &layer) { _dstLayer = layer; }

void CollisionObjectDelegator::setEntity(Entity *pEntity) { _pEntity = pEntity; }

const Entity *CollisionObjectDelegator::entity() const { return _pEntity; }

CollisionObjectDelegator::~CollisionObjectDelegator() {}

void CollisionObjectDelegator::recordCollisionResult(const CollisionResult &result)
{
    _collisionResults.push_back(result);
    if (result.intersect && _collisionCallback)
        _collisionCallback();
}

void CollisionObjectDelegator::cleanResult() { _collisionResults.clear(); }
