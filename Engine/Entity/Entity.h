#pragma once
#include "../../Object/Object.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace vk
{
class CommandBuffer;
class DescriptorSetLayout;
} // namespace vk

namespace Vkbase
{
class Buffer;
}

class Motion;
class Camera;
class CollisionObjectDelegator;

class Entity
{
private:
    struct Deleter 
    {
        void operator()(Entity *pEntity)
        {
            delete pEntity;
        }
    };

    Object _object;
    glm::vec3 _velocity = glm::vec3(0.0f);
    glm::vec3 _tempVelocity = glm::vec3(0.0f);
    glm::vec3 _acceleration = glm::vec3(0.0f);
    const std::string _name;
    const bool _dynamic;
    std::unordered_map<std::string, Motion *> _pMotions;
    CollisionObjectDelegator *_pCollisionObjectDelegator = nullptr;

    inline static std::unordered_map<std::string, std::unique_ptr<Entity, Deleter>> _pEntities = {};
    void updateCollisionObject();
    void updateVelocity(float deltaTime);
    void updatePosition(float deltaTime);
    void update(float deltaTime);

    virtual void init() = 0;

protected:
    Entity(const std::string &name, bool dynamic = true, const Object &object = Object());
    virtual ~Entity();
    CollisionObjectDelegator *collisionObject();
    std::unordered_map<std::string, Motion *> &motions();
    void setCollisionObject(CollisionObjectDelegator *pCollisionObjectDelegator);

public:
    bool dynamic();
    virtual void draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex) const = 0;
    Object &object();
    const Object &object() const;
    const CollisionObjectDelegator *collisionObject() const;

    const std::string &name() const;

    template <typename T> static T &entity(const std::string &name) { return *dynamic_cast<T *>(_pEntities.at(name).get()); }

    virtual std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() = 0;
    static void updateCollisionObjects();
    static void updateAll(float deltaTime);

    glm::vec3 &acceleration();
    const glm::vec3 &acceleration() const;
    glm::vec3 &velocity();
    const glm::vec3 &velocity() const;
    glm::vec3 &tempVelocity();
    const glm::vec3 &tempVelocity() const;

    Motion *addMotion(const std::string &name, Motion *pMotion);
    Motion *motion(const std::string &name);
    void eraseMotion(const std::string &name);

    virtual void updateUBO(const Camera &camera, uint32_t index) const = 0;

    static void drawEntities(const vk::CommandBuffer &commandBuffer, const Camera &camera, uint32_t index);
};