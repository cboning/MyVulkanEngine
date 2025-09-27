#pragma once
#include "../Engine/Physical/Collision/BoxCollisionObject.h"
#include "../Object/Object.h"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>
#include <unordered_map>

namespace vk
{
    class CommandBuffer;
    class DescriptorSetLayout;
}

namespace Vkbase
{
    class Buffer;
}

class Entity
{
private:
    Object _object;
    BoxCollisionObject _collisionObject;
    const std::string _name;
    inline static std::unordered_map<std::string, Entity *> _entities = {};

    virtual void init() = 0;
protected:
    BoxCollisionObject &collisionObject();
public:
    Entity(const std::string &name);
    virtual ~Entity();
    virtual void draw(const vk::CommandBuffer &commandBuffer, uint32_t frameIndex) const = 0;
    Object &object();
    const Object &object() const;
    const std::string &name() const;
    static Entity &entity(const std::string &name);
    virtual std::vector<vk::DescriptorSetLayout> descriptorSetLayouts() = 0;
};