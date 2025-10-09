#include "CollisionObject.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class Object;

class CollisionBox : public CollisionObject
{
    friend class CollisionSystem;
private:
    glm::vec3 _boundBoxSize{};
    glm::mat3 _axes = glm::toMat3(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

public:
    CollisionBox();
    CollisionBox(const glm::vec3 &center, const glm::vec3 &boundBoxSize, const glm::mat3 &axes);
    CollisionBox(const Object &object);
    const glm::vec3 &boundBoxSize() const;
    glm::vec3 halfSize() const;
    const glm::mat3 &axes() const;

    void setBoundBoxSize(const glm::vec3 &boundBoxSize);
    void setAxes(const glm::mat3 &axes);
    void updateWithObject(const Object &object) override;
};