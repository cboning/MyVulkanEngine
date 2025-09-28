#include "Motion.h"

Motion::Motion(Entity &entity) : _entity(entity) {}

Motion::~Motion() {}

Entity &Motion::entity() {
    return _entity;
}
