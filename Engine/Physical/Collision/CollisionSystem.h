#pragma once

#include "../../../Data/Octree.h"
#include "CollisionObjectDelegator.h"
#include <glm/glm.hpp>
#include <string>

enum class CollisionObjectType;

class CollisionObject;
class CollisionBox;
class CollisionCapsule;
class CollisionTriangle;
class Object;

struct CollisionResult;

class CollisionSystem
{
private:
    struct Deleter
    {
        void operator()(CollisionObjectDelegator *pCollisionObjectDelegator);
        void operator()(CollisionObject *pCollisionObject);
    };

    using CollisionObjectDelegatorPtr = std::unique_ptr<CollisionObjectDelegator, Deleter>;

    glm::vec3 _pos;
    glm::vec3 _size;
    
    Octree<CollisionObjectDelegatorPtr>::Ptr _staticCollisionObjects;
    std::vector<CollisionObjectDelegatorPtr> _dynamicCollisionObjects;

    CollisionSystem();
    ~CollisionSystem();

    CollisionResult performCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const;
    CollisionResult performCapsuleVBoxCollisionDetection(const CollisionCapsule &src, const CollisionBox &dst) const;
    CollisionResult performCapsuleVCapsuleCollisionDetection(const CollisionCapsule &src, const CollisionCapsule &dst) const;
    CollisionResult performBoxVBoxCollisionDetection(const CollisionBox &src, const CollisionBox &dst) const;
    CollisionResult performBoxVCapsuleCollisionDetection(const CollisionBox &src, const CollisionCapsule &dst) const;
    CollisionResult performBoxVTriangleCollisionDetection(const CollisionBox &src, const CollisionTriangle &dst) const;
    CollisionResult performTriangleVBoxCollisionDetection(const CollisionTriangle &src, const CollisionBox &dst) const;
    CollisionResult performCapsuleVTriangleCollisionDetection(const CollisionCapsule &src, const CollisionTriangle &dst) const;
    CollisionResult performTriangleVCapsuleCollisionDetection(const CollisionTriangle &src, const CollisionCapsule &dst) const;
    CollisionResult performTriangleVTriangleCollisionDetection(const CollisionTriangle &src, const CollisionTriangle &dst) const;

    void updateWithStaticCollisionObjects(CollisionObjectDelegator &object);
    bool collisionObjectWithOctree(const CollisionObjectDelegator &object, const Octree<CollisionObjectDelegatorPtr> &octree);
    void updateWithDynamicCollisionObjects(CollisionObjectDelegator &object);

public:
    static CollisionSystem &instance();
    template <typename T, typename... Args> CollisionObjectDelegator *createStaticObject(Args &&...args);
    template <typename T, typename... Args> CollisionObjectDelegator *createDynamicObject(Args &&...args);

    void destroyDynamicObject(CollisionObjectDelegator *pCollisionObject);

    void update();
    const Octree<CollisionObjectDelegatorPtr> *octree();

    using CollisionObjectPtr = std::unique_ptr<CollisionObject, Deleter>;
};

template <typename T, typename... Args> CollisionObjectDelegator *CollisionSystem::createStaticObject(Args &&...args)
{
    CollisionObjectDelegator *pCollisionObject = new CollisionObjectDelegator(new T(std::forward<Args>(args)...));
    std::unique_ptr<CollisionObjectDelegator, Deleter> collisionObjectDelegator(pCollisionObject);

    _staticCollisionObjects->addObject(std::move(collisionObjectDelegator));
    return pCollisionObject;
}

template <typename T, typename... Args> CollisionObjectDelegator *CollisionSystem::createDynamicObject(Args &&...args)
{
    CollisionObjectDelegator *pCollisionObject = new CollisionObjectDelegator(new T(std::forward<Args>(args)...));
    std::unique_ptr<CollisionObjectDelegator, Deleter> collisionObjectDelegator(pCollisionObject);
    _dynamicCollisionObjects.push_back(std::move(collisionObjectDelegator));
    return pCollisionObject;
}
