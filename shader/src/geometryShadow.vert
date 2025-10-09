#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

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
    gl_Position = ubo.lightSpaceMatrix * (ubo.model * position);
}