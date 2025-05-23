#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler1;


void main() {

    outColor = vec4(texture(texSampler1, fragTexCoord).rgb * 0.5, 1.0f);// * vec4(fragColor, 1.0);
    // outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

}