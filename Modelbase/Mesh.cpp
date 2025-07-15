#include "Modelbase.h"

namespace Modelbase
{
    Mesh::Mesh(const std::string &name, const std::string &deviceName, const std::vector<ModelData::Vertex> &vertices, const std::vector<uint16_t> &indices, const std::vector<std::vector<std::string>> &textureNames)
        : _device(*dynamic_cast<const Vkbase::Device *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Device, deviceName))), _vertices(vertices), _indices(indices), _textureNames(textureNames), _vertexBuffer(*(new Vkbase::Buffer(getNewBufferName(name + "_Vertex"), deviceName, _vertices.size() * sizeof(_vertices[0]), vk::BufferUsageFlagBits::eVertexBuffer, _vertices.data()))), _indexBuffer(*(new Vkbase::Buffer(getNewBufferName(name + "_Index"), deviceName, _indices.size() * sizeof(_indices[0]), vk::BufferUsageFlagBits::eIndexBuffer, _indices.data()))), _name(name)
    {
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::draw(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const std::vector<vk::DescriptorSet> &descriptorSets) const
    {
        commandBuffer.bindVertexBuffers(0, _vertexBuffer.buffer(), {0});
        commandBuffer.bindIndexBuffer(_indexBuffer.buffer(), 0, vk::IndexType::eUint16);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0, descriptorSets, {});
        commandBuffer.drawIndexed(_indices.size(), 1, 0, 0, 0);
    }

    const std::vector<std::vector<std::string>> &Mesh::textureNames() const
    {
        return _textureNames;
    }

    const std::string Mesh::getNewBufferName(std::string name)
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
};