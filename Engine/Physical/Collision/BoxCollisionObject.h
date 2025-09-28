#pragma once
#include "CollisionObject.h"

class EllipsoidCollisionObject;

class BoxCollisionObject : public CollisionObject
{
private:
    CollisionResult performEllipsoidCollisionDetection(const EllipsoidCollisionObject &target) const;
    CollisionResult performBoxCollisionDetection(const BoxCollisionObject &target) const;

public:
    BoxCollisionObject();
    CollisionResult performCollisionDetection(const CollisionObject &target) const override;
};