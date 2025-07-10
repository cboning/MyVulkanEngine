#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "ModelData.h"


namespace Modelbase
{
    class Bone;

    class Model;

    class Animation
    {
    private:

        float _currentTime;
        float _ticksPerSecond;
        float _duration;

        Model *_pModel;
        std::vector<Bone *> _pBones;
        std::vector<glm::mat4> _transforms;
        void calculateBoneTransform(const ModelData::AssimpNodeData *pNode, glm::mat4 parentTransform);
        Bone *findBone(const std::string &name);
        

    public:
        Animation(aiAnimation *pAnimation, Model *pModel);
        float update(float deltaTime);
        void replay();
        std::vector<glm::mat4> *transformations();
        bool willFinish(float &deltaTime);

    };
};
