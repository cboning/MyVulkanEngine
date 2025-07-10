#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <assimp/vector3.h>
#include <glm/gtc/quaternion.hpp>




class AssimpGLMHelpers
{
public:
    static inline glm::vec3 getGLMVec3(const aiVector3D &vec)
    {
        return glm::vec3(vec.x, vec.y, vec.z);
    };
    static inline glm::vec4 getGLMVec4(const aiColor4D *vec)
    {
        return glm::vec4(vec->r, vec->g, vec->b, vec->a);
    };
    static inline glm::quat getGLMQuat(const aiQuaternion &quat)
    {
        return glm::quat(quat.w, quat.x, quat.y, quat.z);
    };
    static inline glm::mat4 getGLMMat4(const aiMatrix4x4 &mat)
    {
        glm::mat4 to;
		to[0][0] = mat.a1; to[1][0] = mat.a2; to[2][0] = mat.a3; to[3][0] = mat.a4;
		to[0][1] = mat.b1; to[1][1] = mat.b2; to[2][1] = mat.b3; to[3][1] = mat.b4;
		to[0][2] = mat.c1; to[1][2] = mat.c2; to[2][2] = mat.c3; to[3][2] = mat.c4;
		to[0][3] = mat.d1; to[1][3] = mat.d2; to[2][3] = mat.d3; to[3][3] = mat.d4;
        return to;
    };
};
