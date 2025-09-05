#pragma once

#include "../Vkbase/Vkbase.h"
#include "ModelData.h"

namespace Modelbase
{
template <typename T> class Mesh
{
  private:
    const Vkbase::Device &_device;

    std::vector<T> _vertices;
    std::vector<uint16_t> _indices;
    const std::vector<std::vector<std::string>> _textureNames;

    const Vkbase::Buffer &_vertexBuffer;
    const Vkbase::Buffer &_indexBuffer;
    const std::string _name;
    static const std::string getNewBufferWithName(std::string name);

  public:
    Mesh(const std::string &name, const std::string &deviceName, const std::vector<T> &vertices, const std::vector<uint16_t> &indices,
         const std::vector<std::vector<std::string>> &textureNames, const std::string &prefix = "");
    void draw(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const std::vector<vk::DescriptorSet> &descriptorSets) const;
    const std::vector<std::vector<std::string>> &textureNames() const;
    const std::string &name() const;
};

template <typename T>
Mesh<T>::Mesh(const std::string &name, const std::string &deviceName, const std::vector<T> &vertices, const std::vector<uint16_t> &indices,
              const std::vector<std::vector<std::string>> &textureNames, const std::string &prefix)
    : _device(*dynamic_cast<const Vkbase::Device *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Device, deviceName))),
      _vertices(vertices), _indices(indices), _textureNames(textureNames),
      _vertexBuffer(*(Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>(getNewBufferWithName((prefix.empty() ? "" : prefix + "_") + name + "_Vertex"), deviceName,
                                         _vertices.size() * sizeof(_vertices[0]), vk::BufferUsageFlagBits::eVertexBuffer, _vertices.data()))),
      _indexBuffer(*(Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>(getNewBufferWithName((prefix.empty() ? "" : prefix + "_") + name + "_Index"), deviceName,
                                        _indices.size() * sizeof(_indices[0]), vk::BufferUsageFlagBits::eIndexBuffer, _indices.data()))),
      _name(name)
{
}

template <typename T>
void Mesh<T>::draw(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const std::vector<vk::DescriptorSet> &descriptorSets) const
{
    commandBuffer.bindVertexBuffers(0, _vertexBuffer.buffer(), {0});
    commandBuffer.bindIndexBuffer(_indexBuffer.buffer(), 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0, descriptorSets, {});
    commandBuffer.drawIndexed(_indices.size(), 1, 0, 0, 0);
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
}; // namespace Modelbase
