#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragPos;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
}
ubo;

void main() {
    vec4 position = vec4(inPosition, 1.0f);
    gl_Position = ubo.proj * ubo.view * ubo.model * position;
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    fragColor = ubo.color;
    fragPos = (ubo.model * vec4(inPosition, 1.0f)).xyz;
}