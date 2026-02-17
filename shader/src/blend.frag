#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D originColor;
layout(binding = 1) uniform sampler2D bloomColor;

const float gamma = 2.2f;
const float exposure = 1.0f;

void main()
{

    vec4 color = vec4(texture(originColor, fragTexCoord).rgb + texture(bloomColor, fragTexCoord).rgb, 1.0f);
    vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
    // Gamma校正
    mapped = pow(mapped, vec3(1.0 / gamma));
    outColor = vec4(mapped, color.a);
}
