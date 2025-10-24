#version 450

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec4 fragLightSpacePos;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec4 gLightSpacPos;

void main()
{
    gPosition = vec4(1.0f);
    gNormal = vec4(normal, 1.0f);
    gAlbedoSpec.rgb = color;
    gAlbedoSpec.a = 1.0f;
    gLightSpacPos = fragLightSpacePos;
}