#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec4 fragLightSpacePos;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMatrix;
    vec3 color;
}
ubo;

void main()
{
    vec4 position = vec4(inPosition, 1.0f);
    fragLightSpacePos = ubo.lightSpaceMatrix * (ubo.model * position);

    fragPos = vec3(ubo.model * position);
    gl_Position = ubo.proj * ubo.view * vec4(fragPos, 1.0f);
    fragNormal = normalize(mat3(transpose(inverse(ubo.model))) * inNormal);
    fragColor = ubo.color;
}