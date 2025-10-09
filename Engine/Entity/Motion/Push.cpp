#include "Push.h"
#include "../../Entity/Entity.h"

Push::Push() {}

Push::~Push() {}

void Push::update(float) { entity().acceleration() += _acceleration; }

void Push::setAcceleration(const glm::vec3 &acceleration) { _acceleration = acceleration; }

const glm::vec3 &Push::acceleration() const { return _acceleration; }
