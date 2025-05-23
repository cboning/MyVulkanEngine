#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor2;



void main() {
    outColor = vec4(fragTexCoord.x, 1.0, 1.0f, 1.0f);// * vec4(fragColor, 1.0);
    outColor2 = vec4(fragTexCoord.x, 1.0, 1.0f, 1.0f);// * vec4(fragColor, 1.0);
}