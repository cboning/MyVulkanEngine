#pragma once

#include "ModelData.h"
#include "../Vkbase/Vkbase.h"

namespace Modelbase
{
    class Mesh
    {
    private:
        const Vkbase::Device &_device;

        std::vector<ModelData::Vertex> _vertices;
        std::vector<uint16_t> _indices;
        const std::array<std::vector<std::string>, 4> _textureNames;
        std::string _name;
        
        Vkbase::Buffer &_vertexBuffer;
        Vkbase::Buffer &_indexBuffer;
        static const std::string getNewBufferName(std::string name);

    public:
        Mesh(const std::string &name, const std::string &deviceName, const std::vector<ModelData::Vertex> &vertices, const std::vector<uint16_t> &indices, const std::array<std::vector<std::string>, 4> &textureNames);
        ~Mesh();
        void draw(const vk::CommandBuffer &commandBuffer, const Vkbase::Pipeline &pipeline, const std::vector<vk::DescriptorSet> & descriptorSets) const;
        const std::array<std::vector<std::string>, 4> &textureNames() const;
    };
};
