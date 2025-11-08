#pragma once

#include "../Vkbase/Vkbase.h"

namespace Vkbase
{
template <typename T> class Mesh : public Vkbase::VkResourcesDelegator
{
private:
    const VkResourceManagerHolder::WeakReference _device;

    std::vector<T> _vertices;
    std::vector<uint16_t> _indices;
    const std::vector<std::vector<std::string>> _textureNames;

    const VkResourceManagerHolder::WeakReference _vertexBuffer;
    const VkResourceManagerHolder::WeakReference _indexBuffer;
    const std::string _name;
    static const std::string getNewBufferWithName(std::string name);

public:
    Mesh(const std::string &name, const std::string &deviceName, const std::vector<T> &vertices, const std::vector<uint16_t> &indices,
         const std::vector<std::vector<std::string>> &textureNames, const std::string &prefix = "");
    void draw(const VkResourceManagerHolder::WeakReference &commandBuffer,
              const std::vector<std::pair<VkResourceManagerHolder::WeakReference, std::pair<std::string, uint32_t>>> &pDescriptorSets) const;
    const std::vector<std::vector<std::string>> &textureNames() const;
    const std::string &name() const;
    const std::vector<T> &vertices() const;
    const std::vector<uint16_t> &indices() const;
};

template <typename T>
Mesh<T>::Mesh(const std::string &name, const std::string &deviceName, const std::vector<T> &vertices, const std::vector<uint16_t> &indices,
              const std::vector<std::vector<std::string>> &textureNames, const std::string &prefix)
    : _device(Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Device, deviceName)), _vertices(vertices), _indices(indices),
      _textureNames(textureNames),
      _vertexBuffer(createResource<Vkbase::Buffer>(getNewBufferWithName((prefix.empty() ? "" : prefix + "_") + name + "_Vertex"), deviceName,
                                                   _vertices.size() * sizeof(_vertices[0]), vk::BufferUsageFlagBits::eVertexBuffer, _vertices.data())),
      _indexBuffer(createResource<Vkbase::Buffer>(getNewBufferWithName((prefix.empty() ? "" : prefix + "_") + name + "_Index"), deviceName,
                                                  _indices.size() * sizeof(_indices[0]), vk::BufferUsageFlagBits::eIndexBuffer, _indices.data())),
      _name(name)
{
}

template <typename T>
void Mesh<T>::draw(const VkResourceManagerHolder::WeakReference &commandBuffer,
                   const std::vector<std::pair<VkResourceManagerHolder::WeakReference, std::pair<std::string, uint32_t>>> &pDescriptorSets) const
{
    if (auto p = commandBuffer.lock<CommandBuffer>())
    {
        p->bindVertexBuffers(0, _vertexBuffer, {0});
        p->bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint16);
        p->bindDescriptorSets(0, pDescriptorSets, {});
        p->commandBuffer().drawIndexed(_indices.size(), 1, 0, 0, 0);
    }
}

template <typename T> const std::vector<std::vector<std::string>> &Mesh<T>::textureNames() const { return _textureNames; }

template <typename T> const std::string Mesh<T>::getNewBufferWithName(std::string name)
{
    uint32_t count = 0;
    if (!Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Buffer, name).lock())
        return name;

    while (true)
    {
        if (!Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Buffer, name + "_" + std::to_string(count)).lock())
            return name + "_" + std::to_string(count);
        ++count;
    }
}

template <typename T> const std::string &Mesh<T>::name() const { return _name; }
template <typename T> inline const std::vector<T> &Mesh<T>::vertices() const { return _vertices; }
template <typename T> inline const std::vector<uint16_t> &Mesh<T>::indices() const { return _indices; }

}; // namespace Vkbase
