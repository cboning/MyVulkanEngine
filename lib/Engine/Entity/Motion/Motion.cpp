#include "Motion.h"


Motion::Motion() {}

Motion::~Motion() {}

void Motion::setEntity(Entity &entity)
{
    _pEntity = &entity;
}

Entity &Motion::entity() { return *_pEntity; }
