#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler1;


void main() {

    outColor = vec4(texture(texSampler1, fragTexCoord).rgb, 1.0f);
}