#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 normal;

layout(set = 1, binding = 0) uniform sampler2D texSampler1;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor1;

void main() {
    vec4 color = texture(texSampler1, texCoord);
    if (color.a < 0.1)
        discard;
    outColor = color;
    outColor1 = vec4(normal, 1.0f);
}