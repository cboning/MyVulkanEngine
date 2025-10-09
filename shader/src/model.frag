#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec4 fragLightSpacePos;

layout(set = 1, binding = 0) uniform sampler2D texSampler1;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec4 gLightSpacPos;

void main() {
    vec4 color = texture(texSampler1, texCoord);
    if (color.a < 0.1)
        discard;
    gPosition = vec4(fragPos, 1.0f);
    gNormal = vec4(normal, 1.0f);
    gAlbedoSpec.rgb = color.rgb;
    gAlbedoSpec.a = 1.0f;
    gLightSpacPos = fragLightSpacePos;

}