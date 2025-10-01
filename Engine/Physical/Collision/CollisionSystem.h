#pragma once

#include "../../../Data/Octree.h"
#include <glm/glm.hpp>
#include <string>

enum class CollisionObjectType;

class CollisionObjectDelegator;
class CollisionObject;
class Object;

struct CollisionResult;

class CollisionSystem
{
private:
    struct Deleter
    {
        void operator()(CollisionObjectDelegator *pCollisionObjectDelegator);
    };

    using CollisionObjectDelegatorPtr = std::unique_ptr<CollisionObjectDelegator, Deleter>;

    Octree<CollisionObjectDelegatorPtr>::Ptr _staticCollisionObjects;
    std::vector<CollisionObjectDelegatorPtr> _dynamicCollisionObjects;
    glm::vec3 _size;
    glm::vec3 _pos;

    CollisionSystem();
    ~CollisionSystem();

    CollisionResult performCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const;
    CollisionResult performBoxVBoxCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const;
    CollisionResult performBoxVEllipsoidCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const;

    void updateWithStaticCollisionObjects(CollisionObjectDelegator &object);
    bool collisionObjectWithOctree(const CollisionObjectDelegator &object, const Octree<CollisionObjectDelegatorPtr> &octree);
    void updateWithDynamicCollisionObjects(CollisionObjectDelegator &object);

public:
    static CollisionSystem &instance();

    CollisionObjectDelegator *createStaticObject(CollisionObjectType type, const Object &object, const glm::vec3 &positionInBoundBox);
    CollisionObjectDelegator *createDynamicObject(CollisionObjectType type);

    void destroyDynamicObject(CollisionObjectDelegator *pCollisionObject);

    void update();
};