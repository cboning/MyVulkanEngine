#pragma once
#include "CollisionObject.h"

class EllipsoidCollisionObject;

class BoxCollisionObject : public CollisionObject
{
private:
    bool performCollisionDetection(const CollisionObject &target) const override;
    bool performEllipsoidCollisionDetection(const EllipsoidCollisionObject &target) const;
    bool performBoxCollisionDetection(const BoxCollisionObject &target) const;

public:
};