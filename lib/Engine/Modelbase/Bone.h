#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>

#include "ModelData.h"

namespace Modelbase
{
    class Bone
    {
    private:
        std::string _name;
        int _id;

        std::vector<ModelData::KeyPosition> _positions;
        std::vector<ModelData::KeyRotation> _rotations;
        std::vector<ModelData::KeyScale> _scales;
        glm::mat4 _transform;
        int positionIndex(float animationTime);
        int rotationIndex(float animationTime);
        int scaleIndex(float animationTime);
        glm::mat4 updatePosition(float animationTime);
        glm::mat4 updateRotation(float animationTime);
        glm::mat4 updateScale(float animationTime);

        float scaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    public:
        Bone(const std::string &name, int id, const aiNodeAnim *channel);
        const std::string &name() const;
        int id() const;
        void update(float animationTime);
        glm::mat4 transform() const;

    };
};

