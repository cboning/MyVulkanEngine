#pragma once

#include "../Vkbase/Vkbase.h"
#include "ModelData.h"

namespace Modelbase
{
template <typename T> class Mesh : public Vkbase::ResourcesDelegator
{
private:
    const Vkbase::Device &_device;

    std::vector<T> _vertices;
    std::vector<uint16_t> _indices;
    const std::vector<std::vector<std::string>> _textureNames;

    Vkbase::Buffer &_vertexBuffer;
    Vkbase::Buffer &_indexBuffer;
    const std::string _name;
    static const std::string getNewBufferWithName(std::string name);

public:
    Mesh(const std::string &name, const std::string &deviceName, const std::vector<T> &vertices, const std::vector<uint16_t> &indices,
         const std::vector<std::vector<std::string>> &textureNames, const std::string &prefix = "");
    void draw(Vkbase::CommandBuffer *pCommandBuffer, Vkbase::Pipeline &pipeline,
              const std::vector<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> &pDescriptorSets) const;
    const std::vector<std::vector<std::string>> &textureNames() const;
    const std::string &name() const;
    const std::vector<T> &vertices() const;
    const std::vector<uint16_t> &indices() const;
};

template <typename T>
Mesh<T>::Mesh(const std::string &name, const std::string &deviceName, const std::vector<T> &vertices, const std::vector<uint16_t> &indices,
              const std::vector<std::vector<std::string>> &textureNames, const std::string &prefix)
    : _device(*dynamic_cast<const Vkbase::Device *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Device, deviceName))),
      _vertices(vertices), _indices(indices), _textureNames(textureNames),
      _vertexBuffer(*(createResource<Vkbase::Buffer>(getNewBufferWithName((prefix.empty() ? "" : prefix + "_") + name + "_Vertex"), deviceName,
                                                     _vertices.size() * sizeof(_vertices[0]), vk::BufferUsageFlagBits::eVertexBuffer, _vertices.data()))),
      _indexBuffer(*(createResource<Vkbase::Buffer>(getNewBufferWithName((prefix.empty() ? "" : prefix + "_") + name + "_Index"), deviceName,
                                                    _indices.size() * sizeof(_indices[0]), vk::BufferUsageFlagBits::eIndexBuffer, _indices.data()))),
      _name(name)
{
}

template <typename T>
void Mesh<T>::draw(Vkbase::CommandBuffer *pCommandBuffer, Vkbase::Pipeline &pipeline,
                   const std::vector<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> &pDescriptorSets) const
{
    pCommandBuffer->bindPipeline(&pipeline);
    pCommandBuffer->bindVertexBuffers(0, &_vertexBuffer, {0});
    pCommandBuffer->bindIndexBuffer(&_indexBuffer, 0, vk::IndexType::eUint16);
    pCommandBuffer->bindDescriptorSets(0, pDescriptorSets, {});
    pCommandBuffer->commandBuffer().drawIndexed(_indices.size(), 1, 0, 0, 0);
}

template <typename T> const std::vector<std::vector<std::string>> &Mesh<T>::textureNames() const { return _textureNames; }

template <typename T> const std::string Mesh<T>::getNewBufferWithName(std::string name)
{
    uint32_t count = 0;
    if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, name))
        return name;

    while (true)
    {
        if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, name + "_" + std::to_string(count)))
            return name + "_" + std::to_string(count);
        ++count;
    }
}

template <typename T> const std::string &Mesh<T>::name() const { return _name; }
template <typename T> inline const std::vector<T> &Mesh<T>::vertices() const { return _vertices; }
template <typename T> inline const std::vector<uint16_t> &Mesh<T>::indices() const { return _indices; }

}; // namespace Modelbase
