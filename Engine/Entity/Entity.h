#pragma once
#include "../../Object/Object.h"
#include "../Physical/Collision/BoxCollisionObject.h"
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

class Entity
{
private:
    Object _object;
    BoxCollisionObject _collisionObject;
    glm::vec3 _velocity = glm::vec3(0.0f);
    glm::vec3 _acceleration = glm::vec3(0.0f);
    const std::string _name;
    std::unordered_set<Motion *> _pMotions;
    std::vector<CollisionResult> _collisionResults;

    inline static std::unordered_map<std::string, Entity *> _pEntities = {};
    void updateCollisionObject();
    void updateVelocity(float deltaTime);
    void updatePosition(float deltaTime);
    void update(float deltaTime);

    virtual void init() = 0;

protected:
    Entity(const std::string &name);
    virtual ~Entity();
    BoxCollisionObject &collisionObject();
    std::unordered_set<Motion *> &motions();
    std::vector<CollisionResult> &collisionResults();

public:
    virtual void draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex) const = 0;
    Object &object();
    const Object &object() const;
    const std::string &name() const;
    template <typename T> static T &entity(const std::string &name) { return *dynamic_cast<T *>(_pEntities.at(name)); }
    virtual std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() = 0;
    static void updateCollisionObjects();
    static void updateAll(float deltaTime);
    glm::vec3 &acceleration();
    const glm::vec3 &acceleration() const;
    void cleanCollisionResults();
    const std::vector<CollisionResult> &collisionResults() const;
    static void cleanAllCollisionResults();
    glm::vec3 &velocity();
    const glm::vec3 &velocity() const;
};