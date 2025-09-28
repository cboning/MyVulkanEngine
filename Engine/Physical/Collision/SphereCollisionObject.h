#pragma once
#include "CollisionObject.h"
class BoxCollisionObject;

class EllipsoidCollisionObject : public CollisionObject
{
private:
    bool performSphereCollisionDetection(const EllipsoidCollisionObject &target) const;
    bool performBoxCollisionDetection(const BoxCollisionObject &target) const;

public:
    CollisionResult performCollisionDetection(const CollisionObject &target) const override;
};