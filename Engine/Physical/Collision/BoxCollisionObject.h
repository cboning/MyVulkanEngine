#pragma once
#include "CollisionObject.h"

class EllipsoidCollisionObject;

class BoxCollisionObject : public CollisionObject
{
private:
    bool performEllipsoidCollisionDetection(const EllipsoidCollisionObject &target) const;
    bool performBoxCollisionDetection(const BoxCollisionObject &target) const;

public:
    bool performCollisionDetection(const CollisionObject &target) const override;
};