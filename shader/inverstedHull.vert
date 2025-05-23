#version 450

const int MAX_BONES = 250;
const int MAX_BONE_INFLUENCE = 4;
const float screenSizeScale = 0.001;

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 bonesMatrices[MAX_BONES];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;
layout(location = 4) in ivec4 inBoneIds;
layout(location = 5) in vec4 inWeight;

layout(location = 0) out vec2 fragTexCoord;



void main() {
    vec4 position = vec4(0.0f);
    vec3 normal = vec3(0.0f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (inBoneIds[i] == -1)
            continue;
        if (inBoneIds[i] >= MAX_BONES)
        {
            position = vec4(inPosition, 1.0f);
            break;
        }
        position += ubo.bonesMatrices[inBoneIds[i]] * vec4(inPosition, 1.0f) * inWeight[i];
        normal += mat3(ubo.bonesMatrices[inBoneIds[i]]) * inNormal * inWeight[i];
    }
    if (normal == vec3(0.0f))
    {
        position = vec4(inPosition, 1.0f);
        normal = inNormal;
    }
    normal = normalize(normal);

    vec3 cameraPosition = -transpose(mat3(ubo.view)) * ubo.view[3].xyz;
    vec3 cameraFront = -ubo.view[2].xyz;
    vec3 worldNormal = mat3(ubo.model) * normal;
    vec3 projectedNormal = normalize(worldNormal - dot(cameraFront, worldNormal) * cameraFront);
    float screenSpaceScale = screenSizeScale / length(position.xyz - cameraPosition);
    projectedNormal *= screenSpaceScale;
    vec3 reprojectedNormal = normal * dot(normal, projectedNormal);
    position += vec4(reprojectedNormal, 0.0f);

    
    gl_Position = ubo.proj * ubo.view * ubo.model * position;
    fragTexCoord = inTexCoord;
}